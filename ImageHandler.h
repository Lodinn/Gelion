#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <../BPLA/SpectralImage.h>

class ImageHandler : public QObject {
  Q_OBJECT
public:
  ImageHandler(QObject* parent = nullptr);
  ~ImageHandler();
    int set_current_image(int num);
    int get_band_by_wave_lengthl(double wave_length);
    enum show_progress_mode_set { hdr_mode, index_mode };
    show_progress_mode_set show_progress_mode;
    QPixmap changeBrightnessPixmap(QImage &img, qreal brightness);
protected:
    int script_y, script_x; //например, в ImageHandler.h
    QVector<QVector<QVector<double> > > raster;
    Q_INVOKABLE double getByWL(double wl);
public slots:
  void read_envi_hdr(QString fname);
  void append_index_raster(QString for_eval);
  void set_read_file_canceled();
  SpectralImage* current_image() { return image_list[index_current_dataset]; }
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
signals:
  void numbands_obtained(int);
  void reading_progress(int);
  void finished();
  void index_finished(int);
};

#endif // IMAGEHANDLER_H
