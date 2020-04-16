#include "ImageHandler.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QPolygon>
#include <QThread>
#include <QApplication>

ImageHandler::ImageHandler(QObject *parent) {

}

ImageHandler::~ImageHandler() {

}

void ImageHandler::read_envi_hdr(QString fname) {
  QFile hdr_f(fname);
  if(!hdr_f.exists()) return;
  QSettings hdr(fname, QSettings::IniFormat);
  uint32_t h, d, w;
  h = hdr.value("lines").toUInt();
  d = hdr.value("bands").toUInt();
  emit numbands_obtained(d);
  w = hdr.value("samples").toUInt();
  SpectralImage* image = new SpectralImage();
  image->set_image_size(d, h, w);
  uint32_t offset = hdr.value("header offset").toUInt();
  int byteorder = hdr.value("byte order").toInt();
  int dtype = hdr.value("data type").toInt();
  QString interleave = hdr.value("interleave").toString();
  interleave = interleave.toUpper();
  if(interleave != "BIL" || dtype != 4) {
    qDebug() << "Interleave/dtype not implemented yet";
    return;
  }
  QSize size = QSize(w, h);
  if (!hdr_f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
  QTextStream hdrf_in(&hdr_f);
  const QString hdr_content = hdrf_in.readAll();
  auto wlarray_start = hdr_content.indexOf("wavelength");
  auto wlarray = hdr_content.mid(wlarray_start);
  wlarray.replace("\n", "").replace("{", "").replace("}", "");
  QStringList wls_s = wlarray.split(",");
  wls_s[0] = wls_s[0].mid(wls_s[0].indexOf("=") + 1);
  QVector<double> wls;
  for(int i = 0; i < wls_s.count(); i++) {
    wls.append(wls_s.at(i).toDouble());
  }
  image->set_wls(wls);
  hdr_f.close();
  QFileInfo fi(fname);
  auto datpath = fi.path() + "/" + fi.completeBaseName() + ".dat";
  QFile datfile(datpath);
  if(!datfile.exists() || datfile.size() != sizeof(float) * (offset + d*h*w)) {
    qDebug() << "Invalid file size; expected:" << sizeof(float) * (offset + d*h*w) << "bytes, got" << datfile.size();
    return;
  }
  datfile.open(QIODevice::ReadOnly);
  read_file_canceled = false;
  for(int z = 0; z < d; z++) {
    QVector<QVector<double> > slice;
    for(int y = 0; y < h; y++) {
      datfile.seek(offset + sizeof(float) * w * (d * y + z));
      QByteArray buf = datfile.read(sizeof(float) * w);
      QDataStream stream(buf);
      stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
      if(byteorder != 0) stream.setByteOrder(QDataStream::BigEndian);
        else stream.setByteOrder(QDataStream::LittleEndian);
      QVector<double> line;
      for(int x = 0; x < w; x++) {
        double v;
        stream >> v;
        line.append(v);
      }
      slice.append(line);
    }
    image->append_slice(slice);
    emit reading_progress(z);
    if (read_file_canceled) {
      delete image;
      return;
    }
    QApplication::processEvents();
  }
  image_list.append(image);
  index_current_dataset = image_list.count() - 1;
  emit finished();
  datfile.close();
}


void ImageHandler::set_read_file_canceled()
{
    QApplication::processEvents();
    read_file_canceled = 1;
}

