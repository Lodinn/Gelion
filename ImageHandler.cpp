#include "ImageHandler.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QPolygon>
#include <QThread>
#include <QApplication>

ImageHandler::ImageHandler(QObject *parent) {

}

ImageHandler::~ImageHandler() {

}

QVector<double> ImageHandler::getWaveLengths()
{
    return wavelengths[num_data_set];
}

void ImageHandler::read_envi_hdr(QString fname) {

  QFile hdr_f(fname);
  if(!hdr_f.exists()) return;
  QSettings hdr(fname, QSettings::IniFormat);
  height = hdr.value("lines").toUInt();
  depth = hdr.value("bands").toUInt();
  emit numbands_obtained(depth);  // &&&
  QApplication::processEvents();
  width = hdr.value("samples").toUInt();
  uint32_t offset = hdr.value("header offset").toUInt();
  int byteorder = hdr.value("byte order").toInt();
  int dtype = hdr.value("data type").toInt();
  QString interleave = hdr.value("interleave").toString();
  interleave = interleave.toUpper();
  if(interleave != "BIL" || dtype != 4) {
    qDebug() << "Interleave/dtype not implemented yet";
    return;
  }
  slice_size = QSize(width, height);
  if (!hdr_f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QTextStream hdrf_in(&hdr_f);
  const QString hdr_content = hdrf_in.readAll();
  auto wlarray_start = hdr_content.indexOf("wavelength");
  auto wlarray = hdr_content.mid(wlarray_start);
  wlarray.replace("\n", "").replace("{", "").replace("}", "");
  QStringList wls = wlarray.split(",");
  wls[0] = wls[0].mid(wls[0].indexOf("=") + 1);
  for(int i = 0; i < wls.count(); i++) {
      wavelengths[num_data_set].append(wls.at(i).toDouble());
  }
  hdr_f.close();
  QFileInfo fi(fname);
  auto datpath = fi.path() + "/" + fi.completeBaseName() + ".dat";
  QFile datfile(datpath);
  if(!datfile.exists() || datfile.size() != sizeof(float) * (offset + height*depth*width)) {
    qDebug() << "Invalid file size; expected:" << sizeof(float) * (offset + height*depth*width) << "bytes, got" << datfile.size();
    return;
  }
  datfile.open(QIODevice::ReadOnly);
  spectral_image[num_data_set].clear();
  read_file_canceled = 0;
  for(int z = 0; z < depth; z++) {
    QVector<QVector<double> > slice;
    for(int y = 0; y < height; y++) {
      datfile.seek(offset + sizeof(float) * width * (depth * y + z));
      QByteArray buf = datfile.read(sizeof(float) * width);
      QDataStream stream(buf);
      stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
      if(byteorder != 0) stream.setByteOrder(QDataStream::BigEndian);
        else stream.setByteOrder(QDataStream::LittleEndian);
      QVector<double> line;
      for(int x = 0; x < width; x++) {
        double v;
        stream >> v;
        line.append(v);
      }
      slice.append(line);
    }
    spectral_image[num_data_set].append(slice);
    emit reading_progress(z);  // &&&
    if (read_file_canceled == 1) break;
     QApplication::processEvents();
  }
  emit finished();
  datfile.close();
}

QVector<QVector<double> > ImageHandler::get_band(uint16_t band) {
  if(spectral_image[num_data_set].count() < band || band < 0) return QVector<QVector<double> >();
  return spectral_image[num_data_set].at(band);
}

QImage ImageHandler::get_rgb(bool enhance_contrast, int red, int green, int blue) {
  if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
  // Gives the first occurence but not the closest value! Ideally has to be reworked
  // defaul 430 550 570
  double redWLen = wavelengths[num_data_set][red];
  double greenWLen = wavelengths[num_data_set][green];
  double blueWLen = wavelengths[num_data_set][blue];
  int index_blue = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), blueWLen) - wavelengths[num_data_set].begin(),
      index_green = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), greenWLen) - wavelengths[num_data_set].begin(),
      index_red = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), redWLen) - wavelengths[num_data_set].begin();
  QVector<QVector<double> > slice_b = get_band(index_blue), slice_g = get_band(index_green), slice_r = get_band(index_red);
  if(slice_b.isEmpty() || slice_g.isEmpty() || slice_r.isEmpty()) {
    qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE RGB IMAGE";
    return QImage();
  }
  if(slice_b.count() != slice_size.height() || slice_g.count() != slice_size.height() || slice_r.count() != slice_size.height()) {
    qDebug() << "CRITICAL - WRONG SLICE HEIGHT";
    return QImage();
  }
  if(slice_b[0].count() != slice_size.width() || slice_g[0].count() != slice_size.width() || slice_r[0].count() != slice_size.width()) {
    qDebug() << "CRITICAL - WRONG SLICE WIDTH";
    return QImage();
  }

  double max_r = INT_MIN, max_g = INT_MIN, max_b = INT_MIN,
         min_r = INT_MAX, min_g = INT_MAX, min_b = INT_MAX;
  for(int y = 0; y < slice_size.height(); y++) {
    max_r = std::max(max_r, *std::max_element(slice_r[y].begin(), slice_r[y].end()));
    max_g = std::max(max_g, *std::max_element(slice_g[y].begin(), slice_g[y].end()));
    max_b = std::max(max_b, *std::max_element(slice_b[y].begin(), slice_b[y].end()));
    min_r = std::min(min_r, *std::min_element(slice_r[y].begin(), slice_r[y].end()));
    min_g = std::min(min_g, *std::min_element(slice_g[y].begin(), slice_g[y].end()));
    min_b = std::min(min_b, *std::min_element(slice_b[y].begin(), slice_b[y].end()));
  }
  QImage img(slice_size, QImage::Format_RGB32);
  for(int y = 0; y < slice_size.height(); y++) {
    QRgb *im_scLine = reinterpret_cast<QRgb *>(img.scanLine(y));
    for(int x = 0; x < slice_size.width(); x++) {
      if(enhance_contrast) {
        im_scLine[x] = qRgb(qRound((slice_r[y][x] - min_r) / (max_r - min_r) * 255.0),
                            qRound((slice_g[y][x] - min_g) / (max_g - min_g) * 255.0),
                            qRound((slice_b[y][x] - min_b) / (max_b - min_b) * 255.0));
      } else im_scLine[x] = qRgb(qRound(slice_r[y][x] * 255.0), qRound(slice_g[y][x] * 255.0), qRound(slice_b[y][x] * 255.0));
    }
  }
  return img;
}

//QImage ImageHandler::get_rgb(bool enhance_contrast) {
//  if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
//  // Gives the first occurence but not the closest value! Ideally has to be reworked
//  int index_blue = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), 430) - wavelengths[num_data_set].begin(),
//      index_green = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), 550) - wavelengths[num_data_set].begin(),
//      index_red = std::lower_bound(wavelengths[num_data_set].begin(), wavelengths[num_data_set].end(), 570) - wavelengths[num_data_set].begin();
//  QVector<QVector<double> > slice_b = get_band(index_blue), slice_g = get_band(index_green), slice_r = get_band(index_red);
//  if(slice_b.isEmpty() || slice_g.isEmpty() || slice_r.isEmpty()) {
//    qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE RGB IMAGE";
//    return QImage();
//  }
//  if(slice_b.count() != slice_size.height() || slice_g.count() != slice_size.height() || slice_r.count() != slice_size.height()) {
//    qDebug() << "CRITICAL - WRONG SLICE HEIGHT";
//    return QImage();
//  }
//  if(slice_b[0].count() != slice_size.width() || slice_g[0].count() != slice_size.width() || slice_r[0].count() != slice_size.width()) {
//    qDebug() << "CRITICAL - WRONG SLICE WIDTH";
//    return QImage();
//  }

//  double max_r = INT_MIN, max_g = INT_MIN, max_b = INT_MIN,
//         min_r = INT_MAX, min_g = INT_MAX, min_b = INT_MAX;
//  for(int y = 0; y < slice_size.height(); y++) {
//    max_r = std::max(max_r, *std::max_element(slice_r[y].begin(), slice_r[y].end()));
//    max_g = std::max(max_g, *std::max_element(slice_g[y].begin(), slice_g[y].end()));
//    max_b = std::max(max_b, *std::max_element(slice_b[y].begin(), slice_b[y].end()));
//    min_r = std::min(min_r, *std::min_element(slice_r[y].begin(), slice_r[y].end()));
//    min_g = std::min(min_g, *std::min_element(slice_g[y].begin(), slice_g[y].end()));
//    min_b = std::min(min_b, *std::min_element(slice_b[y].begin(), slice_b[y].end()));
//  }
//  QImage img(slice_size, QImage::Format_RGB32);
//  for(int y = 0; y < slice_size.height(); y++) {
//    QRgb *im_scLine = reinterpret_cast<QRgb *>(img.scanLine(y));
//    for(int x = 0; x < slice_size.width(); x++) {
//      if(enhance_contrast) {
//        im_scLine[x] = qRgb(qRound((slice_r[y][x] - min_r) / (max_r - min_r) * 255.0),
//                            qRound((slice_g[y][x] - min_g) / (max_g - min_g) * 255.0),
//                            qRound((slice_b[y][x] - min_b) / (max_b - min_b) * 255.0));
//      } else im_scLine[x] = qRgb(qRound(slice_r[y][x] * 255.0), qRound(slice_g[y][x] * 255.0), qRound(slice_b[y][x] * 255.0));
//    }
//  }
//  return img;
//}

QVector<QPointF> ImageHandler::get_profile(QPoint p) {
  //Check if p is inside the image area
    QPolygon poly;
    poly << QPoint(0,0) << QPoint(width,0) << QPoint(height,width) << QPoint(0,height) << QPoint(0,0);
    if (!poly.containsPoint(p, Qt::OddEvenFill)) return QVector<QPointF>();

  //get point with coordinates z (from 0 to max), y, x (from p) from spectral_image and return it
    int x = p.x();    int y = p.y();    QVector<QPointF> v;     QPointF f;
    for(int z = 0; z < depth; z++) {
        f.rx() = wavelengths[num_data_set][z];
        f.ry() = spectral_image[num_data_set].at(z).at(y).at(x);
        v.append(f);
    }
    return v;
}

int ImageHandler::get_bands_count()
{
    return depth;
}

void ImageHandler::set_read_file_canceled()
{
    QApplication::processEvents();
    read_file_canceled = 1;
}

QList<QString> ImageHandler::getWaveLengthsList(int precision)
{
    QList<QString> strlist;
    strlist.append(QString("RGB [R:%1 nm,G:%2 nm, B:%3 nm]").arg(430).arg(550).arg(570));
    for(int z = 0; z < depth; z++)
        strlist.append(QString("номер %1 * %2  нм").arg(z + 1, 3).
                       arg(wavelengths[num_data_set][z], 7, 'f', precision));
    return strlist;
}
