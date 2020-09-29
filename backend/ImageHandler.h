#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QSlider>
#include <QIcon>
#include <QMessageBox>
#include "SpectralImage.h"

class ImageHandler : public QObject {
  Q_OBJECT
public:
  ImageHandler(QObject* parent = nullptr);
  ~ImageHandler();
//! returns true on success (current index set to the requested one), false otherwise
    bool set_current_image(int image_list_index);
    int get_current_image_num() { return index_current_dataset; }
    QString get_regular_expression(QString input);
    int get_band_by_wave_length(double wave_length);
    enum show_progress_mode_set { hdr_mode, index_mode };
    show_progress_mode_set show_progress_mode;
    QPixmap changeBrightnessPixmap(QImage &img, qreal brightness);
    double zero_brightness = 3.5;
    double scale_brightness = zero_brightness / 33.0;
    double zero_brightness_pos = 0.3;
    void set_brightness_to_slider(QSlider *slider, double brightness);
    QImage get_REAL_current_image();
    double get_new_brightness(QSlider *slider, int value);
//    void save_settings_all_images(QStringList &save_file_names);
    QString getHDRfileNameConvertedFromJPG(QString jpg_name);  // hdr файл, ассоциированный с jpg, при необходимости файл создается
    QIcon load_icon_from_file(QString &fname, double rotation);
    QString get_icon_for_tooltip(QString &fname);
    QString getWritableLocation();  // активный каталог проекта
    QString getWritableLocationByNum(int num);  // каталог проекта номер num
    void showWarningMessageBox(QMessageBox *mb, QString text);
    QString get_hidden_folder();
    QString get_hidden_folder_by_num(int num);
    void save_to_hidden_folder();
    void close_all_datasets() {
        for (int i=0; i<image_list.count();i++) { delete image_list[i]; }
        image_list.clear();
        index_current_dataset = -1; }

protected:
    int script_y, script_x; // например, в ImageHandler.h
//    QVector<QVector<QVector<double> > >* raster;
    Q_INVOKABLE double getByWL(double wl);
    QString mainIconFileName = "/main.png";
    QString mainIconFileNameTT = "/main_tt.png";
public slots:
  void read_envi_hdr(QString fname);
  void append_index_raster(QString for_eval);
  void set_read_file_canceled();
  SpectralImage* current_image() { return image_list[index_current_dataset]; }
  int get_images_count() { return image_list.count(); }
  QStringList get_image_file_names() {
    QStringList strlist;
    foreach(SpectralImage *img, image_list) strlist.append(img->get_file_name());
    return strlist;
  }
private:
  bool read_file_canceled = false;
  int index_current_dataset = -1;
  QList<SpectralImage*> image_list;
  void save_slice(QString fname, QVector<QVector<double> > slice);
  QString rgbConvertedEnvi = "Data converted from JPG PNG";
  QString dataFX10eEnvi = "FX10e data";
  void createHdrDatFilesFromJPG(QString jpg_name, QString hdr_name);
  double rgb_to_float = 255.;
  QString hidden_folder = "/do not change this folder/";
public:
  SpectralImage* get_image(int num) {
      if (num < 0 || num > image_list.count()-1) return nullptr;
      return image_list[num]; }
  bool data_file_reading = false;  // переключатель режима чтения
signals:
  void numbands_obtained(int);
  void reading_progress(int);
  void finished();
  void index_finished(int);
  void change_current_image(int old_index_dataset);
};

#endif // IMAGEHANDLER_H
