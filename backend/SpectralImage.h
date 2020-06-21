#ifndef SPECTRALIMAGE_H
#define SPECTRALIMAGE_H

#include <QObject>
#include <QVector>
#include <QSize>
#include <QImage>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace J09 {
  struct histogramType {
    double min = INT_MAX;
    double max = INT_MIN;
    double sum = .0;
    int hcount = 256;
    QVector<double > vx, vy;  // статистика / совместимость с qCustomPlot
    double lower = .0, upper = .0, sum_of_part = 100.;
    double wl380 = 380.;
    double wl781 = 781.;
    double brightness = 1.;
    double ___plotmouse = .005*3;
    double ___sbrightness = .3;  // относительное положение 1. на слайдере
    bool imgPreview = true;
    bool colorized = true;
    double rotation = .0;  // previw picture rotate
    int rgb_preview = 0;  // 0 - index, 1 - rgb, 2 - black\white, 3 - mask
  };  // histogramType
  struct plotStyle {
    QColor color;
    int style;
  };  // spectralColorType
}
QT_END_NAMESPACE

class SpectralImage : public QObject {
  Q_OBJECT
public:
  explicit SpectralImage(QObject *parent = nullptr);
  QString get_file_name() { return fname; }
  QVector<QVector<QVector<double> > > get_raster() { return img; }
  QSize get_raster_x_y_size() { return slice_size; }
  void  calculateHistogram(bool full, uint16_t num);
  double getBrightness(uint32_t num) { return indexBrightness.at(num); }
public slots:

  QVector<QVector<double> > get_band(uint16_t band);

  QImage get_grayscale(bool enhance_contrast = false, uint16_t band = 0);
  QImage get_rgb(bool enhance_contrast = false, int red = 0, int green = 0, int blue = 0);
  QImage get_index_rgb(bool enhance_contrast = false, bool colorized = true, int num_index = 0);
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
  QVector<QVector<QVector<quint8> > > mask;  // маска изображения, 0 - нет, больше нуля - маска
  int mask_index = -1;  // текущий номер маски

  QVector<double> wavelengths;
  QSize slice_size;
  uint32_t depth, height, width;
  QVector<QImage> indexImages;  // нулевой QImage содержит дефолтную RGB
  QVector<QPair<QString, QString> > indexNameFormulaList;  // списко индексов "наименование,формула"
  QVector<double > indexBrightness;
  double default_brightness = 1.5;
  int current_slice = -1;
  QRgb get_rainbow_RGB(double Wavelength);
public:
  QVector<J09::histogramType> histogram;  // статистика гистограммы
  double wl380 = 380.0;
  double wl781 = 781.0;
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
