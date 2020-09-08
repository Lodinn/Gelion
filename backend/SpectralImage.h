#ifndef SPECTRALIMAGE_H
#define SPECTRALIMAGE_H

#include <QObject>
#include <QVector>
#include <QSize>
#include <QImage>
#include <QDebug>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace J09 {
  struct RGB_CANNELS {
    double red = 641.0;
    double green = 550.0;
    double blue = 460.0;
  };  // RGB_CANNELS
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
    double ___plotmouse = 3;  // область притяжения курсора в пикселях
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
  struct indexType {
      QListWidgetItem *listwidget = nullptr;
      QString name;  // наименование
      QString formula;  // формула
      bool rgb = false;
      QVector<QVector<double> > rgb_r;  // rgb red
      QVector<QVector<double> > rgb_g;  // rgb green
      QVector<QVector<double> > rgb_b;  // rgb blue
      QVector<QVector<double> > slice;  // индексный массив
      QImage img;  // растровое изображение
      J09::histogramType histogram;  // статистика гистограммы
  };  // indexType
  struct globalSettingsType {
      bool load_resent_project = true;  // при запуске загружать последний проект
      bool save_project_settings = false;  // сохранять настройки проекта
      bool restore_project_settings = false;  // восстанавливать настройки проекта
      int resent_files_count = 3;  // &&& количество последних файлов
      bool save_not_checked_roi = true;  // сохранять в настройках неотмеченные области интереса
      int zobject_dock_size_w = 450;  // размеры окна профиля, пиксели
      int zobject_dock_size_h = 150;  // размеры окна профиля, пиксели
      bool zobjects_prof_rainbow_show = true;  // заливка спектральных диапазонов
      bool zobjects_prof_deviation_show = true;  // заливка стандартного отклонения
      double zobject_plot_xAxis_lower = 390.;  // спектральный диапазон, меньшее
      double zobject_plot_xAxis_upper = 1010.;  // спектральный диапазон, большее
      double zobject_plot_yAxis_lower = 0.;  // коэффициент отражения, меньшее
      double zobject_plot_yAxis_upper = 1.5;  // коэффициент отражения, большее
      int std_dev_brush_color_red = 200;
      int std_dev_brush_color_green = 200;
      int std_dev_brush_color_blue = 200;
      int std_dev_brush_color_transparency = 200;
      double main_rgb_rotate_start = 90.;
      double main_rgb_scale_start = 3.;
  };
  struct maskRecordType {  // пакет данных об изображении - Маска
      bool checked = true;
      QString title;  // наименование
      QString formula;  // общая формула алгоритма
      bool invers = false;  // инверсное изображение
      QStringList formula_step_by_step;  // общая формула алгоритма разбитая на составляющие
      QVector<QVector<int8_t> > mask;  // маска
      QImage img;
      double brightness = 3.;
      double rotation = .0;
  };
}
QT_END_NAMESPACE

class slice_magic : public QObject {
public:
    explicit slice_magic(QObject *parent = nullptr);
    void save(QDataStream &stream);
//    void load(QDataStream &stream);
    void set_slice(QVector<QVector<double> > sl) { slice = sl; }
    QVector<QVector<double> > get_slice() { return slice; }
    void set_mask(QVector<QVector<int8_t> > m) { mask = m; }
    void set_channel_num(int num) { ch_num = num; }

    int get_channel_num() { return ch_num; }
    void set_wave_length(double wl) { wave_length = wl; }
    void set_brightness(double br) { brightness = br;  h.brightness = br; }
    double get_brightness() { return brightness; }
    void set_rotation(double r) { rotation = r; }
    void set_title(QString a_title);
    QString get_title() { return title; }
    void calculateHistogram(bool full);
    J09::RGB_CANNELS get_RGB_defaults() { return rgb_wl; }  // default wave lengths
    void set_LW_item(QListWidgetItem *lwItem) {
        listwidget = lwItem;
        listwidget->setText(get_title());
        if (index) listwidget->setToolTip(get_formula());
        listwidget->setIcon(get_icon());
    }
    void set_slice_size(double h, double w) { slice_size = QSize(w, h); }
    void set_red(QVector<QVector<double> > r) { rgb_r = r; }
    void set_green(QVector<QVector<double> > g) { rgb_g = g; }
    void set_blue(QVector<QVector<double> > b) { rgb_b = b; }
    void set_image(QImage im) {image = im; }
    QImage get_image() { return image; }
    void set_RGB() { rgb = true; }
    void set_Index() { index = true; }
    void set_MASK() { is_mask = true; }
    QImage get_rgb(bool enhance_contrast);
    QImage get_index_rgb(bool enhance_contrast, bool colorized);
    void set_formula(QString f) { formula = f; }
    QString get_formula() { return formula; }
    void set_inverse(bool inv) { inverse = inv; }
    void set_formula_step(QStringList strList) { formula_step_by_step = strList; }
    void create_icon();
    QIcon get_icon() { return icon; }
    J09::histogramType h;  // статистика гистограммы
private:
    QRgb get_rainbow_RGB(double Wavelength);

    QListWidgetItem *listwidget = nullptr;
    QString title = "";  // наименование канала в списке выбора listwidget
    double wave_length;  // wave length
    int ch_num; // номер канала
    double brightness = 1.;  // яркость
    double rotation = .0;  // previw picture rotate
    QVector<QVector<double> > slice;    // массив данных об изображении
    QVector<QVector<double> > rgb_r;    // rgb red
    QVector<QVector<double> > rgb_g;    // rgb green
    QVector<QVector<double> > rgb_b;    // rgb blue
    QVector<QVector<int8_t> > mask;  // маска
    QImage image;                       // растровое изображение (черно\белое, при RGB цветное)
    QIcon icon;                         // иконка для списка выбора
    bool rgb = false;
    bool index = false;
    bool is_mask = false;
    QString formula = "";  // формула индекса \ маски
    // общая формула алгоритма составной маскиразбитая на составляющие
    QStringList formula_step_by_step;
    bool inverse = false;            // инверсное изображение \ маска
    J09::RGB_CANNELS rgb_wl;  // default wave lengths
    QSize slice_size;
};

class SpectralImage : public QObject {
  Q_OBJECT
public:
  explicit SpectralImage(QObject *parent = nullptr);
  ~SpectralImage();
  enum dataType { dtBase, dtRGB, dtFX10e };
  dataType datatype = dataType::dtBase;
  QString get_file_name() { return fname; }
  void save_icon_to_file(QPixmap &pixmap, QSize &size);

  QVector<QVector<QVector<double> > >* get_raster() { return &img; }
  QSize get_raster_x_y_size() { return slice_size; }
  void  calculateHistogram(bool full, uint16_t num);
  double get_index_brightness(uint32_t num) {
//      return indexBrightness.at(num);
      return indexes[num]->get_brightness();
  }
  QString get_y_axis_title_for_plot(bool short_title);  // наименование единиц измерения
  QString mainIconFileName = "/main.png";

public slots:
  void append_slice(QVector<QVector<double> > slice);
  void append_RGB();
  int append_index(QVector<QVector<double> > slice);
  void append_mask(J09::maskRecordType *msk);

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
public:
  void set_LW_item(QListWidgetItem *lwItem, int num);
  QImage get_current_image();
  void set_formula_for_index(double r, QString i_title, QString i_formula);
  slice_magic *get_current_index() { return indexes[current_slice - depth]; }
private:
  QVector<slice_magic *> bands;  // каналы
  QVector<slice_magic *> indexes;  // RGB, индексные изображения
  QVector<slice_magic *> masks;  // маски
// index order (from outer to inner): z, y, x
  QVector<QVector<QVector<double> > > img;  // img свыше последнего канала содержит индексные изображения
  QVector<J09::indexType> img_additional;  // массив индексных изображений , включая RGB
  QVector<J09::maskRecordType *> rec_masks;  // массив масочных изображений
  QVector<QImage> indexImages;  // нулевой QImage содержит дефолтную RGB
  QVector<QPair<QString, QString> > indexNameFormulaList;  // списко индексов "наименование,формула"
  QVector<double > indexBrightness;

  int current_slice = -1;         // текущий номер массива данных, 0..deep-1 - каналы, далее RGB, потом индексы
  int current_mask_index = -1;  // текущий номер маски

  QVector<double> wavelengths;
  QSize slice_size;
  uint32_t depth, height, width;
  double default_brightness = 2.5;

  QRgb get_rainbow_RGB(double Wavelength);
  int get_band_by_wave_length(double wave_l);

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
      if (num < 0 || num >  depth + indexes.count() - 1) return;
//      if (num < 0 || num >  indexBrightness.count() - 1) return;
      current_slice = num; }
  int get_current_slice() { return current_slice; }
  double get_current_brightness();
  void set_current_brightness(double value);
  QPair<QString, QString> get_current_formula();

  int setCurrentMaskIndex(int num);
  int getCurrentMaskIndex() { return current_mask_index; }
  J09::maskRecordType *getCurrentMask();
  J09::maskRecordType *getMask(int num);
  QImage current_mask_image();
  QImage get_mask_image(int num);
  QImage get_mask_image(J09::maskRecordType &msk);
  int getMasksCount() { return rec_masks.count(); }
  void deleteAllMasks();
};

#endif // SPECTRALIMAGE_H
