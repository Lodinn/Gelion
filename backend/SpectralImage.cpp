#include "SpectralImage.h"
#include <QPoint>
#include <QPolygon>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QtMath>

SpectralImage::SpectralImage(QObject *parent) : QObject(parent) {
    //    img.clear();
}

QVector<QVector<double> > SpectralImage::get_band(uint16_t band){
  if(img.count() < band) return QVector<QVector<double> >();
  return img.at(band);
}

QImage SpectralImage::get_grayscale(bool enhance_contrast, uint16_t band) {
  if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
  QVector<QVector<double> > slice = get_band(band);
  if(slice.isEmpty()) {
    qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE RGB IMAGE";
    return QImage();
  }
  double max = INT_MIN, min = INT_MAX;
  for(int y = 0; y < slice_size.height(); y++) {
    max = std::max(max, *std::max_element(slice[y].begin(), slice[y].end()));
    min = std::min(min, *std::min_element(slice[y].begin(), slice[y].end()));
  }
  QImage img(slice_size, QImage::Format_Grayscale8);
  for(int y = 0; y < slice_size.height(); y++) {
    uchar *im_scLine = reinterpret_cast<uchar *>(img.scanLine(y));
    for(int x = 0; x < slice_size.width(); x++) {
      if(enhance_contrast) {
        im_scLine[x] = qRound((slice[y][x] - min) / (max - min) * 255.0);
      } else im_scLine[x] = qRound(slice[y][x] * 255.0);
    }
  }
  return img;
}

QImage SpectralImage::get_rgb(bool enhance_contrast, int red, int green, int blue) {
  if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
  // Gives the first occurence but not the closest value! Ideally has to be reworked
  // defaul 430 550 570
  double redWLen = wavelengths[red];
  double greenWLen = wavelengths[green];
  double blueWLen = wavelengths[blue];
  int index_blue = std::lower_bound(wavelengths.begin(), wavelengths.end(), blueWLen) - wavelengths.begin(),
      index_green = std::lower_bound(wavelengths.begin(), wavelengths.end(), greenWLen) - wavelengths.begin(),
      index_red = std::lower_bound(wavelengths.begin(), wavelengths.end(), redWLen) - wavelengths.begin();
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

QImage SpectralImage::get_index_rgb(bool enhance_contrast, bool colorized, int num_index)
{
    if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
    if(num_index < 0 || num_index > img.length() - 1) {
        qDebug() << "CRITICAL - WRONG INDEX";
        return QImage();
    }

    QVector<QVector<double> > slice_index = get_band(num_index);

    if(slice_index.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QImage();
    }
    if(slice_index.count() != slice_size.height() || slice_index[0].count() != slice_size.width()) {
      qDebug() << "CRITICAL! - WRONG SLICE HEIGHT OR WIDTH";
      return QImage();
    }
    double max_bw = INT_MIN, min_bw = INT_MAX;
    for(int y = 0; y < slice_size.height(); y++) {
      max_bw = std::max(max_bw, *std::max_element(slice_index[y].begin(), slice_index[y].end()));
      min_bw = std::min(min_bw, *std::min_element(slice_index[y].begin(), slice_index[y].end()));
    }

    // NDVI -0.621685 0.915984

    QImage index_img(slice_size, QImage::Format_RGB32);

    if (colorized)  // с использованием цветовой схемы
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                double virtual_wlen = wl380 + (slice_index[y][x] - min_bw)
                        * (wl781 - wl380) / (max_bw - min_bw);
                im_scLine[x] = get_rainbow_RGB(virtual_wlen);
            }  // for int x
        }  // for int y
    else  // черно-белое изображение
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                if(enhance_contrast) {
                    double d = (slice_index[y][x] - min_bw) / (max_bw - min_bw) * 255.0;
                    im_scLine[x] =  qRgb(qRound(d), qRound(d), qRound(d));
                } else {
                    double dd = slice_index[y][x] * 255.0;
                    im_scLine[x] = qRgb(qRound(dd), qRound(dd), qRound(dd));
                }  // if
            }    // for int x
        }  // for int y

    return index_img;
}

QRgb SpectralImage::get_rainbow_RGB(double Wavelength)
{
    static double Gamma = 0.80;
    static double IntensityMax = 255;

    double factor;
    double Red,Green,Blue;

    if((Wavelength >= 380) && (Wavelength<440)){
        Red = -(Wavelength - 440) / (440 - 380);
        Green = 0.0;
        Blue = 1.0;
    }else if((Wavelength >= 440) && (Wavelength<490)){
        Red = 0.0;
        Green = (Wavelength - 440) / (490 - 440);
        Blue = 1.0;
    }else if((Wavelength >= 490) && (Wavelength<510)){
        Red = 0.0;
        Green = 1.0;
        Blue = -(Wavelength - 510) / (510 - 490);
    }else if((Wavelength >= 510) && (Wavelength<580)){
        Red = (Wavelength - 510) / (580 - 510);
        Green = 1.0;
        Blue = 0.0;
    }else if((Wavelength >= 580) && (Wavelength<645)){
        Red = 1.0;
        Green = -(Wavelength - 645) / (645 - 580);
        Blue = 0.0;
    }else if((Wavelength >= 645) && (Wavelength<781)){
        Red = 1.0;
        Green = 0.0;
        Blue = 0.0;
    }else{
        Red = 0.0;
        Green = 0.0;
        Blue = 0.0;
    }

// Let the intensity fall off near the vision limits

    if((Wavelength >= 380) && (Wavelength<420)){
        factor = 0.3 + 0.7*(Wavelength - 380) / (420 - 380);
    }else if((Wavelength >= 420) && (Wavelength<701)){
        factor = 1.0;
    }else if((Wavelength >= 701) && (Wavelength<781)){
        factor = 0.3 + 0.7*(780 - Wavelength)  / (780 - 700);
    }else{
        factor = 0.0;
    }

    int r = 0, g = 0, b = 0;

// Don't want 0^x = 1 for x <> 0
    const double zero = 0.00000001;
    if (Red < zero) r = 0;
    else r = qRound(IntensityMax * qPow(Red * factor, Gamma));
    r = qMin(255, r);
    if (Green < zero) g = 0;
    else g = qRound(IntensityMax * qPow(Green * factor, Gamma));
    g = qMin(255, g);
    if (Blue < zero) b = 0;
    else b = qRound(IntensityMax * qPow(Blue * factor, Gamma));
    b = qMin(255, b);

    return qRgb(r,g,b);
}

QVector<QPointF> SpectralImage::get_profile(QPoint p) {
  //Check if p is inside the image area
  QPolygon poly;
  poly << QPoint(0,0) << QPoint(width, 0) << QPoint(height, width) << QPoint(0, height) << QPoint(0, 0);
  if (!poly.containsPoint(p, Qt::OddEvenFill)) return QVector<QPointF>();

  //get point with coordinates z (from 0 to max), y, x (from p) from spectral_image and return it
  int x = p.x();    int y = p.y();    QVector<QPointF> v;     QPointF f;
  for(int z = 0; z < depth; z++) {
      f.rx() = wavelengths[z];
      f.ry() = img.at(z).at(y).at(x);
      v.append(f);
  }
  return v;
}


QList<QString> SpectralImage::get_wl_list(int precision) {
  QList<QString> strlist;
  for(int z = 0; z < depth; z++)
      strlist.append(QString("номер %1 * %2 нм").arg(z + 1, 3).
                     arg(wavelengths[z], 7, 'f', precision));
  return strlist;
}

void SpectralImage::append_slice(QVector<QVector<double> > slice) {
  if(slice.count() != slice_size.height() || slice.count() <= 0 || slice.at(0).count() != slice_size.width()) {
    qDebug() << "Bad slice size! Current slice count:" << img.count() << "expected:" << slice_size;
    return;
  }
  img.append(slice);
  indexBrightness.append(default_brightness);
  J09::histogramType hg;  // статистика гистограммы
  hg.brightness = default_brightness;
  histogram.append(hg);
  uint16_t num = img.count() - 1;

  if (num > depth - 1) calculateHistogram(true, num);

}

void SpectralImage::calculateHistogram(bool full, uint16_t num)
{
    QVector<QVector<double> > slice = get_band(num);
    J09::histogramType hg = histogram[num];  // статистика гистограммы

    int h = slice.count();  int w = slice.at(0).count();
    if (full) {
      hg.max = INT_MIN;  hg.min = INT_MAX;

      for(int y = 0; y < h; y++) {
        hg.max = std::max(hg.max, *std::max_element(slice[y].begin(), slice[y].end()));
        hg.min = std::min(hg.min, *std::min_element(slice[y].begin(), slice[y].end()));
      }  // for
      if (hg.max == INT_MIN || hg.min == INT_MAX) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      hg.lower = hg.min;  hg.upper = hg.max;
      hg.sum = .0;  hg.sum_of_part = 100.;
      hg.vx.resize(hg.hcount);  hg.vy.resize(hg.hcount);  hg.vy.fill(0.);
      if (std::abs(hg.max - hg.min) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT - all the values are the same; min/max:" << hg.min << hg.max;
        return;
      }  // error

      double k = (hg.hcount-1.)/(hg.max-hg.min);
      if (std::abs(k) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      for (int i=0; i<hg.hcount; i++) hg.vx[i]=hg.min+i/k;
      for (int y=0; y<h; y++)
          for (int x=0; x<w; x++){
              int num_int = qRound((slice[y][x]-hg.min)*k);
              if (num_int < 0) num_int = 0;
              if (num_int > hg.hcount - 1) num_int = hg.hcount - 1;
              hg.vy[num_int]++;
              hg.sum++;
          }  // for
      histogram[num] = hg;
      return;
    }  // if (full)
    hg.sum_of_part = 0.;
    for(int i=0;i<hg.hcount-1;i++) {
        if (hg.vx[i] < hg.lower) continue;
        if (hg.vx[i] > hg.upper) continue;
        hg.sum_of_part += hg.vy[i];
    }  // for
    if (qAbs(hg.sum) < .000001) {
        hg.sum_of_part = .0;  return;
    }  // if
    hg.sum_of_part = 100.*hg.sum_of_part/hg.sum;

    histogram[num] = hg;
}

QString SpectralImage::get_y_axis_title_for_plot(bool short_title)
{
    QString result("?????");
    if (short_title)
        switch (datatype) {
        case dataType::dtBase : result = "к-т отражения";  break;
        case dataType::dtRGB : result = "яркость";  break;
        case dataType::dtFX10e : result = "яркость";  break;
        }  // switch
    else
        switch (datatype) {
        case dataType::dtBase : result = "коэффициент отражения ( r )";  break;
        case dataType::dtRGB : result = "яркость ( r )";  break;
        case dataType::dtFX10e : result = "яркость ( r )";  break;
        }  // switch

    return result;
}

void SpectralImage::append_additional_image(QImage image, QString index_name, QString index_formula)
{
    indexImages.append(image);
    indexNameFormulaList.append(qMakePair(index_name,index_formula));
    indexBrightness.append(default_brightness);
}

QImage SpectralImage::get_additional_image(int num)
{
    if (num < 0 && num > indexImages.count() - 1)
        return QImage();
    return indexImages.at(num);
}

void SpectralImage::save_additional_slices(QString binfilename)
{
    if (depth == img.count()) return;
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    QFile file(writableLocation + "/" + binfilename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
    for (int i=depth; i<img.count(); i++)
        out << img.at(i);
    file.close();
}

void SpectralImage::load_additional_slices(QString binfilename)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QFile file(writableLocation + "/" + binfilename);
    if (!file.exists()) return;
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    in.setByteOrder(QDataStream::LittleEndian);
    for (int i = 0; i < indexNameFormulaList.count() - 1; i++) {
        QVector<QVector<double> >  additional_slices;
        in >> additional_slices;
        img.append(additional_slices);
    }  // for
    file.close();
}

void SpectralImage::save_images(QString pngfilename)
{
    if (indexImages.count() == 0) return;
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo file(fname);
    writableLocation += "/" + file.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    for (int i=0; i<indexImages.count(); i++) {
        indexImages.at(i).save(QString("%1/%2%3")
                               .arg(writableLocation).arg(i).arg(pngfilename));
    }  // for
}

void SpectralImage::load_images(QString pngfilename)
{
    indexImages.clear();
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo file(fname);
    writableLocation += "/" + file.completeBaseName();
    for (int i=0; i<indexNameFormulaList.count(); i++) {
        QImage image;
        image.load(QString("%1/%2%3")
                   .arg(writableLocation).arg(i).arg(pngfilename));
        indexImages.append(image);
    }  // for
}

void SpectralImage::save_formulas(QString images_name)
{
    if (indexNameFormulaList.count() == 0) return;
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    QFile file(writableLocation + "/" + images_name);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
    out << indexNameFormulaList;
    file.close();
}

bool SpectralImage::load_formulas(QString images_name)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QFile file(writableLocation + "/" + images_name);
    if (!file.exists()) return false;
    indexNameFormulaList.clear();
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> indexNameFormulaList;
    file.close();
    return true;
}

void SpectralImage::save_brightness(QString images_brightness)
{
    if (indexBrightness.count() == 0) return;
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QDir dir(writableLocation);
    if (!dir.exists()) dir.mkpath(".");
    QFile file(writableLocation + "/" + images_brightness);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
    out << indexBrightness;
    file.close();
}

void SpectralImage::load_brightness(QString images_brightness)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo fi(fname);
    writableLocation += "/" + fi.completeBaseName();
    QFile file(writableLocation + "/" + images_brightness);
    if (!file.exists()) return;
    indexBrightness.clear();
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> indexBrightness;
    file.close();
}

double SpectralImage::get_current_brightness()
{
    if (current_slice < 0) return 3.0;
    if (current_slice > indexBrightness.count() - 1) return 3.0;
    return indexBrightness.at(current_slice);
}

void SpectralImage::set_current_brightness(double value)
{
    if (current_slice < 0 || current_slice > indexBrightness.count() - 1) return;
    indexBrightness[current_slice] = value;
}

QPair<QString, QString> SpectralImage::get_current_formula()
{
    return indexNameFormulaList.at(current_slice - depth);
}

void SpectralImage::append_mask(J09::maskRecordType *msk)
{
    masks.append(msk);
    current_mask_index = masks.count() - 1;
}

int SpectralImage::setCurrentMaskIndex(int num)
{
    if (num < 0 || num > masks.count() - 1) return -1;
    current_mask_index = num;
    return current_mask_index;
}

J09::maskRecordType *SpectralImage::getCurrentMask()
{
    return masks.at(current_mask_index);
}

J09::maskRecordType *SpectralImage::getMask(int num)
{
    if (num < 0 || num > masks.count() - 1) return nullptr;
    return masks.at(num);
}

QImage SpectralImage::current_mask_image()
{
    J09::maskRecordType *cm = getCurrentMask();
    QSize slice_size(cm->mask[0].count(), cm->mask.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                mask_img.setPixel(x, y, cm->mask[y][x]);
            }  // for

        return mask_img;
}

QImage SpectralImage::get_mask_image(int num)
{
    J09::maskRecordType *mask = getMask(num);
    if (mask == nullptr) {
        qDebug() << "MASK READING PROBLEM !!!";
        return QImage();
    }
    QSize slice_size(mask->mask[0].count(), mask->mask.count());
    QImage mask_img(slice_size, QImage::Format_Mono);

        for(int y = 0; y < slice_size.height(); y++)
            for(int x = 0; x < slice_size.width(); x++) {
                mask_img.setPixel(x, y, mask->mask[y][x]);
            }  // for

        return mask_img;
}
