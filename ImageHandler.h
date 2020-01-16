#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QVector>
#include <QImage>

class ImageHandler : public QObject {
  Q_OBJECT
public:
  ImageHandler(QObject* parent = nullptr);
  ~ImageHandler();
public slots:
  void read_envi_hdr(QString fname);
  QVector<QVector<double> > get_band(uint16_t band);
  QImage get_rgb(bool enhance_contrast = false);
  QVector<QPointF> get_profile(QPoint p);
  int get_bands_count();
  void set_read_file_canceled();
private:
  // index order (from outer to inner): z, y, x
  int read_file_canceled = 0;
  int num_data_set = 0;
  QVector<QVector<QVector<double> > > spectral_image[2];  // &&& 2
  QVector<double> wavelengths[2];  // &&& 2
  QSize slice_size;
  uint32_t height, depth, width;
signals:
  void numbands_obtained(int);
  void reading_progress(int);
  void finished();
};

#endif // IMAGEHANDLER_H
