#include "ImageHandler.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QPolygon>
#include <QThread>
#include <QApplication>
#include <QRegularExpression>
#include <QJSEngine>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>

ImageHandler::ImageHandler(QObject *parent) { Q_UNUSED(parent) }

ImageHandler::~ImageHandler() {}

bool ImageHandler::set_current_image(int image_list_index) {
  if ((image_list_index < 0) ||
      (image_list_index > image_list.length() - 1)) return false;
  index_current_dataset = image_list_index;
  return true;
}

QString ImageHandler::get_regular_expression(QString input)
{
    QString sanitized = input.replace('[', '(').replace(']', ')').
            replace('r', 'R').replace('v', 'R');
    sanitized.replace(QRegularExpression("[^explognR^* /+\\-().\\d<>& ]"), "");
    //FIXME: expand to expressions
    sanitized.replace(QRegularExpression("\\b(R\\d+)\\^([\\d|.]+)\\b"), "Math.pow(\\1, \\2)");
    QString for_eval = sanitized.replace(QRegularExpression("R(\\d+)"), "getByWL(\\1)").replace("ln", "log");
    return for_eval;
}

int ImageHandler::get_band_by_wave_length(double wavelength) {

/*<<<<<<< HEAD
  int result = 0;
  QVector<double> wls = current_image()->wls();
  if (wavelength < wls[0]) return 0;
  if (wavelength > wls[wls.count()-1]) return wls.count()-1;
// проверка попадания на максимальную длину волны и минимальное число каналов
  if (wls.length() == 1)  return 0;
  double d = wls[wls.length()-1] - wls[wls.length()-2];
  if (qAbs(wavelength - wls[wls.length()-1]) < d * .5) return wls.length() - 1;
=======*/

  SpectralImage* image = current_image();
  QVector<double> wls = image->wls();
  if (wavelength < wls[0]) return 0;
  int result = wls.length() - 1;
  for (int ch = 0; ch < wls.length() - 1; ch++) {
    if ((wavelength >= wls[ch]) &&
        (wavelength < wls[ch+1])) {
      result = wavelength - wls[ch] < wls[ch+1] - wavelength ? ch : ch + 1;
      return result;
    }  // if
  }  // for
  return result; //if nothing was found, return the last channel
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

void ImageHandler::set_brightness_to_slider(QSlider *slider, double brightness)
{
    double range = slider->maximum() - slider->minimum();
    double delta = brightness - zero_brightness;
    double value = delta / scale_brightness;
    value += range * zero_brightness_pos;
    int a = qRound(value);
    slider->setValue(a);
}

QImage ImageHandler::get_REAL_current_image()
{
    int num = current_image()->get_current_slice();
    uint32_t depth = current_image()->get_bands_count();
    if (num < depth)
        return current_image()->get_rgb(true,num,num,num);
    return current_image()->get_additional_image(num - depth);
}

double ImageHandler::get_new_brightness(QSlider *slider, int value)
{
    double range = slider->maximum() - slider->minimum();
    double delta = value - range * zero_brightness_pos;
    double result = zero_brightness + delta * scale_brightness;
    current_image()->set_current_brightness(result);
    return result;
}

void ImageHandler::save_settings_all_images(QStringList &save_file_names)
{
    for (int i = 0; i < image_list.count(); i++) {
        set_current_image(i);
        current_image()->save_additional_slices(save_file_names.at(0));
        current_image()->save_images(save_file_names.at(1));
        current_image()->save_formulas(save_file_names.at(2));
        current_image()->save_brightness(save_file_names.at(3));
    } // for
}

QString ImageHandler::getHDRfileNameConvertedFromJPG(QString jpg_name)
{
    QFileInfo info(jpg_name);
    QString hdrFileName = info.completeBaseName() + ".hdr";
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    writableLocation += "/" + info.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    hdrFileName = writableLocation + "/" + hdrFileName;
    if(QFileInfo::exists(hdrFileName)) return hdrFileName;
    createHdrDatFilesFromJPG(jpg_name, hdrFileName);
    return hdrFileName;
}

void ImageHandler::createHdrDatFilesFromJPG(QString jpg_name, QString hdr_name)
{
    QImage image(jpg_name);
    image.convertToFormat(QImage::Format_RGB32);

    QFileInfo info(hdr_name);
    QString dat_name = info.path() + "/" + info.completeBaseName() + ".dat";

    J09::RGB_CANNELS rgb_default;
    QStringList hdr_list;
    hdr_list << "ENVI" << "description = {" << rgbConvertedEnvi << "}"
             << QString("samples = %1").arg(image.width()) << QString("lines = %1").arg(image.height())
             << "bands = 3" << "header offset = 0" << "file type = ENVI" << "data type = 4" << "interleave = BIL"
             << "sensor type = unknown" << "byte order = 0" << "wavelength = {" << QString("%1,").arg(rgb_default.blue, 0, 'f', 2)
             << QString("%1,").arg(rgb_default.green, 0, 'f', 2) << QString("%1").arg(rgb_default.red, 0, 'f', 2) << "}";


    QFile hdrfile(hdr_name);
    hdrfile.remove();
    hdrfile.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream hdrstream(&hdrfile);
    hdrstream.setCodec("UTF-8");
    hdrstream.setGenerateByteOrderMark(true);
    foreach(QString str, hdr_list) hdrstream << str << endl;
    hdrstream.flush();
    hdrfile.close();

    QFile datfile(dat_name);
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );
    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);  // byte order = 0

    for(int row=0; row<image.height(); row++) {
        QRgb *rowData = (QRgb*)image.scanLine(row);
        for(int band=0; band<3; band++)
            for(int col=0; col<image.width(); col++) {
                QRgb pixelData = rowData[col];
                int pos = band*image.width()+col;
                switch (band) {
                case 0: {
                    float blue = qBlue(pixelData) / rgb_to_float;  datstream << blue;
                    break; }
                case 1: {
                    float green = qGreen(pixelData) / rgb_to_float;  datstream << green;
                    break; }
                case 2: {
                    float red = qRed(pixelData) / rgb_to_float;  datstream << red;
                    break; }
                }  // switch
            }  // for col
    }  // for row
    datfile.close();

}

double ImageHandler::getByWL(double wl) {
  int bn = get_band_by_wave_length(wl); //логика для получения номера канала по длине волны
  return current_image()->get_raster()->at(bn).at(script_y).at(script_x);
}

void ImageHandler::append_index_raster(QString for_eval) {
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
      output_array[script_y][script_x] = precompiled.call().toNumber();
    }  // for
    emit reading_progress(script_y);
    if (read_file_canceled) return;
    QApplication::processEvents();
  }  // for
  current_image()->append_slice(output_array);

  emit index_finished(current_image()->get_raster()->length() - 1);
}

void ImageHandler::save_slice(QString fname, QVector<QVector<double> > slice)
{
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(addin_path);  if (!dir.exists()) dir.mkpath(addin_path);
    QFile file(addin_path + "/" + fname);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
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
  if(interleave != "BIL") {
    qDebug() << "Interleave not implemented yet";
    return;
  }
  if (!hdr_f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QTextStream hdrf_in(&hdr_f);
  const QString hdr_content = hdrf_in.readAll();
  auto rgb_start = hdr_content.indexOf(rgbConvertedEnvi);  // определение типа данных
  if (rgb_start != -1) image->datatype = SpectralImage::dtRGB;
  rgb_start = hdr_content.indexOf(dataFX10eEnvi);  // определение типа данных
  if (rgb_start != -1) image->datatype = SpectralImage::dtFX10e;
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
  auto datpath = fi.path() + "/" + fi.completeBaseName();
  QFile datfile("");

  switch (image->datatype) {
  case SpectralImage::dtBase :
      datfile.setFileName(datpath + ".dat");
      break;
  case SpectralImage::dtRGB :
      datfile.setFileName(datpath + ".dat");
      break;
  case SpectralImage::dtFX10e :
      datfile.setFileName(datpath + ".img");  //FIXME: check the entire QDir::EntryList for the corresponding file
      break;
  }  // switch

  if (datfile.fileName().isEmpty()) {
      qDebug() << "WRONG format ENVI file";  return;
  }  // if

  uint16_t sizeof_data = -1;
  switch(dtype) {
    case 1:
      sizeof_data = 1;
      break;
    case 2:
      sizeof_data = 2;
      break;
    case 3:
      sizeof_data = 4;
      break;
    case 4:
      sizeof_data = 4;
      break;
    case 5:
      sizeof_data = 8;
      break;
    case 12:
      sizeof_data = 2;
      break;
    case 13:
      sizeof_data = 4;
      break;
    case 14:
      sizeof_data = 8;
      break;
    case 15:
      sizeof_data = 8;
      break;
    default:
      qDebug() << "dtype not implemented";
      return;
  }
//  if(!datfile.exists()) datfile.setFileName(datpath + ".img"); //FIXME: check the entire QDir::EntryList for the corresponding file

  if(!datfile.exists() || datfile.size() != sizeof_data * (offset + d*h*w)) {
    qDebug() << "Invalid file size; expected:" << sizeof_data * (offset + d*h*w) << "bytes, got"
             << datfile.size() << "from " << datfile.fileName();
    return;
  }  // if
  datfile.open(QIODevice::ReadOnly);
  read_file_canceled = false;
//<<<<<<< HEAD
//  double v;     int16_t u16;
//=======
  uint8_t v_byte;
  int16_t v_int16;
  int32_t v_int32;
  double v_double;
  uint16_t v_uint16;
  uint32_t v_uint32;
  int64_t v_int64;
  uint64_t v_uint64;
//>>>>>>> bcba70a07b408bca5587070546373c4b11034040
  for(int z = 0; z < d; z++) {
    QVector<QVector<double> > slice;
    for(int y = 0; y < h; y++) {
      datfile.seek(offset + sizeof_data * w * (d * y + z));
      QByteArray buf = datfile.read(sizeof_data * w);
      QDataStream stream(buf);
      if(dtype == 4) stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
      if(byteorder != 0) stream.setByteOrder(QDataStream::BigEndian);
        else stream.setByteOrder(QDataStream::LittleEndian);

      QVector<double> line;
      for(int x = 0; x < w; x++) {
        switch(dtype) {
          case 1:
            stream >> v_byte;
            line << v_byte;
            break;
          case 2:
            stream >> v_int16;
            line << v_int16;
            break;
          case 3:
            stream >> v_int32;
            line << v_int32;
            break;
          case 4:
//<<<<<<< HEAD
//          case 5: { //the only difference is setFloatingPointPrecision above
//            stream >> v;
//            line.append(v);
//            break; }
//          case 12: { //     stream >> reinterpret_cast<uint16_t&>(v);
//            stream >> u16;
//            line.append(u16);
//            break; }
//=======
          case 5: //the only difference is setFloatingPointPrecision above
            stream >> v_double;
            line << v_double;
            break;
          case 12:
            stream >> v_uint16;
            line << v_uint16;
            break;
//>>>>>>> bcba70a07b408bca5587070546373c4b11034040
          case 13:
            stream >> v_uint32;
            line << v_uint32;
            break;
          case 14:
            stream >> v_int64;
            line << v_int64;
            break;
          case 15:
            stream >> v_uint64;
            line << v_uint64;
            break;
        }
//<<<<<<< HEAD
//        line.append(v);
//=======
//>>>>>>> bcba70a07b408bca5587070546373c4b11034040
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
// connect(im_handler, SIGNAL(finished()), this, SLOT(add_envi_hdr_pixmap()));
  datfile.close();
}

void ImageHandler::set_read_file_canceled()
{
    QApplication::processEvents();
    read_file_canceled = true;
}

