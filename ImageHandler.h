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
public slots:
  void read_envi_hdr(QString fname);
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
signals:
  void numbands_obtained(int);
  void reading_progress(int);
  void finished();
};

#endif // IMAGEHANDLER_H
