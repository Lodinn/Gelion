#ifndef SPECTRALIMAGE_H
#define SPECTRALIMAGE_H

#include <QObject>
#include <QVector>
#include <QSize>
#include <QImage>
#include <QDebug>

class SpectralImage : public QObject {
  Q_OBJECT
public:
  explicit SpectralImage(QObject *parent = nullptr);
  QString get_file_name() { return fname; }
  QVector<QVector<QVector<double> > > get_raster() { return img; }
  QSize get_raster_x_y_size() { return slice_size; }
public slots:
  QVector<QVector<double> > get_band(uint16_t band);
  QImage get_grayscale(bool enhance_contrast = false, uint16_t band = 0);
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
  QVector<QVector<QVector<double> > > img;  // img свыше последнего канала содержит индексные изображения
  QVector<double> wavelengths;
  QSize slice_size;
  uint32_t depth, height, width;
  QVector<QImage> indexImages;  // нулевой QImage содержит дефолтную RGB
  QVector<QPair<QString, QString> > indexNameFormulaList;  // списко индексов "наименование,формула"
  QVector<double > indexBrightness;
  double default_brightness = 3.5;
  int current_slice = -1;
public:
  QString fname;
  void append_additional_image(QImage image, QString index_name, QString index_formula);
  QImage get_additional_image(int num);
  int get_image_count() { return indexImages.count(); }

  void save_additional_slices(QString binfilename);
  void save_images(QString pngfilename);
  void save_formulas(QString images_name);
  void save_brightness(QString  images_brightness);

  void load_additional_slices(QString binfilename);
  void load_images(QString pngfilename);
  bool load_formulas(QString images_name);  // проверка на наличие ВСЕГО набора
  void load_brightness(QString  images_brightness);

  void set_current_slice(int num) {
      if (num < 0 || num >  indexBrightness.count() - 1) return;
      current_slice = num; }
  int get_current_slice() { return current_slice; }
  double get_current_brightness();
  void set_current_brightness(double value);
  QPair<QString, QString> get_current_formula();

signals:

};

#endif // SPECTRALIMAGE_H
