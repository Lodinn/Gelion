#include "SpectralImage.h"
#include <QPoint>
#include <QPolygon>
#include <QDebug>

SpectralImage::SpectralImage(QObject *parent) : QObject(parent) {
  img.clear();
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

QImage SpectralImage::get_index_rgb(bool enhance_contrast, int num_index)
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

    QImage index_img(slice_size, QImage::Format_RGB32);

    for(int y = 0; y < slice_size.height(); y++) {
        QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
        for(int x = 0; x < slice_size.width(); x++) {
            if(enhance_contrast) {
                double d = (slice_index[y][x] - min_bw) / (max_bw - min_bw) * 255.0;
                im_scLine[x] = qRgb(qRound(d), qRound(d), qRound(d));
            } else {
                double dd = slice_index[y][x] * 255.0;
                im_scLine[x] = qRgb(qRound(dd), qRound(dd), qRound(dd));
            }  // if
        }    // for int x
    }  // for int y

    return index_img;
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
}
