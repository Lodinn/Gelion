#ifndef SPECTRALIMAGE_H
#define SPECTRALIMAGE_H

#include <QObject>
#include <QVector>
#include <QSize>
#include <QImage>

class SpectralImage : public QObject {
  Q_OBJECT
public:
  explicit SpectralImage(QObject *parent = nullptr);
    QString get_file_name() { return fname; }
    QVector<QVector<QVector<double> > > get_raster() { return img; }
    QSize get_raster_x_y_size() { return slice_size; }
public slots:
  QVector<QVector<double> > get_band(uint16_t band);
  QImage get_rgb(bool enhance_contrast = false, int red = 0, int green = 0, int blue = 0);
  QImage get_index_rgb(bool enhance_contrast = false, int num_index = 0);
  QVector<QPointF> get_profile(QPoint p);
  inline uint32_t get_bands_count() { return depth; }
  QList<QString> get_wl_list(int precision);
  inline QVector<double> wls() { return wavelengths; }
  void set_image_size(uint32_t d, uint32_t h, uint32_t w) {
    depth = d;
    height = h;
    width = w;
    slice_size = QSize(w, h);
    img.clear();
  }
  void set_wls(QVector<double> wls) { wavelengths = wls; }
  void append_slice(QVector<QVector<double> > slice);

private:
  // index order (from outer to inner): z, y, x
  QVector<QVector<QVector<double> > > img;
  QVector<double> wavelengths;
  QSize slice_size;
  uint32_t depth, height, width;

public:
  QString fname;

//  friend void ImageHandler::read_envi_hdr(QString);
signals:
};

#endif // SPECTRALIMAGE_H
