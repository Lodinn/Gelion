#include "ImageHandler.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QPolygon>
#include <QThread>
#include <QApplication>
#include <QJSEngine>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>

ImageHandler::ImageHandler(QObject *parent) { Q_UNUSED(parent) }

ImageHandler::~ImageHandler() {}

int ImageHandler::set_current_image(int num)
{
    int result;
    if ((num < 0) || (num > image_list.length() - 1)) result = -1;
    else { result = num; index_current_dataset = result; }
    return result;
}

int ImageHandler::get_band_by_wave_lengthl(double wave_length)
{
    int result = - 1;
    SpectralImage* image = current_image();
    QVector<double> wls = image->wls();
    for (int ch = 0; ch < wls.length() - 1; ch++)
        if ((wave_length >= wls[ch]) && (wave_length < wls[ch+1])) {
            result = ch;  return result;
        }  // if
    return result;
}

QPixmap ImageHandler::changeBrightnessPixmap(QImage &img, qreal brightness)
{
    QColor color;
    int h, s, v;
    qreal t_brightnessValue = brightness;
    for(int row = 0; row < img.height(); ++row) {
        unsigned int * data = (unsigned int *) img.scanLine(row);
        for(int col = 0; col < img.width(); ++col) {
            unsigned int & pix = data[col];
            color.setRgb(qRed(pix), qGreen(pix), qBlue(pix));
            color.getHsv(&h, &s, &v);
            v *= t_brightnessValue; // значение, в которое надо увеличить яркость
            v = qMin(v, 255);
            color.setHsv(h, s, v);
// сохраняем изменённый пиксель
            pix = qRgba(color.red(), color.green(), color.blue(), qAlpha(pix));
        }  // for
    }  // for
    return QPixmap::fromImage(img);
}

double ImageHandler::getByWL(double wl) {
  int bn = get_band_by_wave_lengthl(wl); //логика для получения номера канала по длине волны
  return raster.at(bn).at(script_y).at(script_x);
}

void ImageHandler::append_index_raster(QString for_eval)
{
    raster = current_image()->get_raster();
    read_file_canceled = false;
    show_progress_mode = ImageHandler::index_mode;
    QJSEngine engine;
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    QJSValue js_imagehandler = engine.newQObject(this);
    QJSValue js_get_by_wl =  js_imagehandler.property("getByWL");
    engine.globalObject().setProperty("getByWL", js_get_by_wl);
    QSize size = current_image()->get_raster_x_y_size();
    int w = size.width();  int h = size.height();
    emit numbands_obtained(h);
    auto precompiled = engine.evaluate("(function(){ return " + for_eval + "})");
    QVector<QVector<double> > output_array = QVector<QVector<double> >(h , QVector<double>(w));
    for(script_y = 0; script_y < h; script_y++) {
      for(script_x = 0; script_x < w; script_x++) {
  //        qDebug() << precompiled.call().toNumber() << script_y << script_x;
        output_array[script_y][script_x] = precompiled.call().toNumber();
      }  // for
      emit reading_progress(script_y);
      if (read_file_canceled) return;
      QApplication::processEvents();
    }  // for
    qDebug() << "in" << current_image()->get_raster().length();
    current_image()->append_slice(output_array);

//    save_slice("aaa.bin", output_array);

    qDebug() << "out" << current_image()->get_raster().length();
    emit index_finished(current_image()->get_raster().length() - 1);
}

void ImageHandler::save_slice(QString fname, QVector<QVector<double> > slice)
{
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(addin_path);  if (!dir.exists()) dir.mkpath(addin_path);
    QFile file(addin_path + "/" + fname);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << slice;
    file.close();
}

void ImageHandler::read_envi_hdr(QString fname) {
  show_progress_mode = ImageHandler::hdr_mode;
  QFile hdr_f(fname);
  if(!hdr_f.exists()) return;
  QSettings hdr(fname, QSettings::IniFormat);
  uint32_t h, d, w;
  h = hdr.value("lines").toUInt();
  d = hdr.value("bands").toUInt();
  w = hdr.value("samples").toUInt();
  emit numbands_obtained(d);
  SpectralImage* image = new SpectralImage();
  image->fname = fname;
  image->set_image_size(d, h, w);
  uint32_t offset = hdr.value("header offset").toUInt();
  int byteorder = hdr.value("byte order").toInt();
  int dtype = hdr.value("data type").toInt();
  QString interleave = hdr.value("interleave").toString();
  interleave = interleave.toUpper();
  if(interleave != "BIL" || dtype != 4) {
    qDebug() << "Interleave/dtype not implemented yet";
    return;
  }
//  QSize size = QSize(w, h);
  if (!hdr_f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QTextStream hdrf_in(&hdr_f);
  const QString hdr_content = hdrf_in.readAll();
  auto wlarray_start = hdr_content.indexOf("wavelength");
  auto wlarray = hdr_content.mid(wlarray_start);
  wlarray.replace("\n", "").replace("{", "").replace("}", "");
  QStringList wls_s = wlarray.split(",");
  wls_s[0] = wls_s[0].mid(wls_s[0].indexOf("=") + 1);
  QVector<double> wls;
  for(int i = 0; i < wls_s.count(); i++) {
    wls.append(wls_s.at(i).toDouble());
  }
  image->set_wls(wls);
  hdr_f.close();
  QFileInfo fi(fname);
  auto datpath = fi.path() + "/" + fi.completeBaseName() + ".dat";
  QFile datfile(datpath);
  if(!datfile.exists() || datfile.size() != sizeof(float) * (offset + d*h*w)) {
    qDebug() << "Invalid file size; expected:" << sizeof(float) * (offset + d*h*w) << "bytes, got" << datfile.size();
    return;
  }
  datfile.open(QIODevice::ReadOnly);
  read_file_canceled = false;
  for(int z = 0; z < d; z++) {
    QVector<QVector<double> > slice;
    for(int y = 0; y < h; y++) {
      datfile.seek(offset + sizeof(float) * w * (d * y + z));
      QByteArray buf = datfile.read(sizeof(float) * w);
      QDataStream stream(buf);
      stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
      if(byteorder != 0) stream.setByteOrder(QDataStream::BigEndian);
        else stream.setByteOrder(QDataStream::LittleEndian);
      QVector<double> line;
      for(int x = 0; x < w; x++) {
        double v;
        stream >> v;
        line.append(v);
      }
      slice.append(line);
    }
    image->append_slice(slice);
    emit reading_progress(z);
    if (read_file_canceled) {
      delete image;
      return;
    }
    QApplication::processEvents();
  }
  image_list.append(image);
  index_current_dataset = image_list.count() - 1;
  emit finished();
  datfile.close();
}


void ImageHandler::set_read_file_canceled()
{
    QApplication::processEvents();
    read_file_canceled = true;
}

