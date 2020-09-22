#include "SpectralImage.h"
#include <QPoint>
#include <QPolygon>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QtMath>
#include <QPixmap>
#include <QIcon>

SpectralImage::SpectralImage(QObject *parent) : QObject(parent) {}

SpectralImage::~SpectralImage()
{
    foreach(slice_magic *sm, bands) delete sm;
}

void SpectralImage::save_icon_to_file(QPixmap &pixmap, QSize size)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString icon_fname = writableLocation + "/" + info.completeBaseName() + mainIconFileName;
    if (QFile(icon_fname).exists()) return;
    QPixmap small_pixmap = pixmap.scaled(size);
    small_pixmap.save(icon_fname);
}

QVector<QVector<double> > SpectralImage::get_band(uint16_t num){
  if (num < depth) return bands[num]->get_slice();
  else return indexes[num - depth]->get_slice();
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

QImage SpectralImage::get_index_rgb(bool enhance_contrast, bool colorized, int num_index)
{
    if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();
    if(num_index < 0 || num_index > bands.count()+indexes.count() - 1) {
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

    // NDVI -0.621685 0.915984

    QImage index_img(slice_size, QImage::Format_RGB32);

    if (colorized)  // с использованием цветовой схемы
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                double virtual_wlen = wl380 + (slice_index[y][x] - min_bw)
                        * (wl781 - wl380) / (max_bw - min_bw);
                im_scLine[x] = get_rainbow_RGB(virtual_wlen);
            }  // for int x
        }  // for int y
    else  // черно-белое изображение
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                if(enhance_contrast) {
                    double d = (slice_index[y][x] - min_bw) / (max_bw - min_bw) * 255.0;
                    im_scLine[x] =  qRgb(qRound(d), qRound(d), qRound(d));
                } else {
                    double dd = slice_index[y][x] * 255.0;
                    im_scLine[x] = qRgb(qRound(dd), qRound(dd), qRound(dd));
                }  // if
            }    // for int x
        }  // for int y

    return index_img;
}

QRgb SpectralImage::get_rainbow_RGB(double Wavelength)
{
    static double Gamma = 0.80;
    static double IntensityMax = 255;

    double factor;
    double Red,Green,Blue;

    if((Wavelength >= 380) && (Wavelength<440)){
        Red = -(Wavelength - 440) / (440 - 380);
        Green = 0.0;
        Blue = 1.0;
    }else if((Wavelength >= 440) && (Wavelength<490)){
        Red = 0.0;
        Green = (Wavelength - 440) / (490 - 440);
        Blue = 1.0;
    }else if((Wavelength >= 490) && (Wavelength<510)){
        Red = 0.0;
        Green = 1.0;
        Blue = -(Wavelength - 510) / (510 - 490);
    }else if((Wavelength >= 510) && (Wavelength<580)){
        Red = (Wavelength - 510) / (580 - 510);
        Green = 1.0;
        Blue = 0.0;
    }else if((Wavelength >= 580) && (Wavelength<645)){
        Red = 1.0;
        Green = -(Wavelength - 645) / (645 - 580);
        Blue = 0.0;
    }else if((Wavelength >= 645) && (Wavelength<781)){
        Red = 1.0;
        Green = 0.0;
        Blue = 0.0;
    }else{
        Red = 0.0;
        Green = 0.0;
        Blue = 0.0;
    }

// Let the intensity fall off near the vision limits

    if((Wavelength >= 380) && (Wavelength<420)){
        factor = 0.3 + 0.7*(Wavelength - 380) / (420 - 380);
    }else if((Wavelength >= 420) && (Wavelength<701)){
        factor = 1.0;
    }else if((Wavelength >= 701) && (Wavelength<781)){
        factor = 0.3 + 0.7*(780 - Wavelength)  / (780 - 700);
    }else{
        factor = 0.0;
    }

    int r = 0, g = 0, b = 0;

// Don't want 0^x = 1 for x <> 0
    const double zero = 0.00000001;
    if (Red < zero) r = 0;
    else r = qRound(IntensityMax * qPow(Red * factor, Gamma));
    r = qMin(255, r);
    if (Green < zero) g = 0;
    else g = qRound(IntensityMax * qPow(Green * factor, Gamma));
    g = qMin(255, g);
    if (Blue < zero) b = 0;
    else b = qRound(IntensityMax * qPow(Blue * factor, Gamma));
    b = qMin(255, b);

    return qRgb(r,g,b);
}

int SpectralImage::get_band_by_wave_length(double wave_l)
{
    if (wave_l < wavelengths[0]) return 0;  // with left bound return first channel
    int result = wavelengths.length() - 1;
    for (int ch = 0; ch < wavelengths.length() - 1; ch++) {
      if ((wave_l >= wavelengths[ch]) &&
          (wave_l < wavelengths[ch+1])) {
        result = wave_l - wavelengths[ch] < wavelengths[ch+1] - wave_l ? ch : ch + 1;
        return result;
      }  // if
    }  // for
    return result; //if nothing was found, return the last channel
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
//      f.ry() = img.at(z).at(y).at(x);
      f.ry() = get_band(z).at(y).at(x);
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
    qDebug() << "Bad slice size! Current slice count:" << bands.count() << "expected:" << slice_size;
    return;
  }
  slice_magic *sm = new slice_magic();  bands.append(sm);  sm->set_data_file_name(fname);
  sm->set_slice(slice);
  sm->set_slice_size(height,width);
  sm->set_channel_num(bands.count());
  int ch_num = sm->get_channel_num();
  sm->set_wave_length(wavelengths[ch_num-1]);
  sm->set_brightness(default_brightness);
  QImage im = sm->get_index_rgb(true,false);
  sm->set_image(im);
  sm->create_icon();

//  if (ch_num > depth) sm->calculateHistogram(true);

  sm->set_Band();
  sm->set_title("");

}

void SpectralImage::append_RGB()
{
    for (int i=0; i<indexes.count(); i++) {
        slice_magic *sm = indexes[i];
        delete sm;
    }
    indexes.clear();
    slice_magic *sm = new slice_magic();  indexes.append(sm);  sm->set_data_file_name(fname);
    sm->set_slice_size(height,width);
    // 84 - 641nm 53 - 550nm 22 - 460nm
    J09::RGB_CANNELS rgb_default = sm->get_RGB_defaults();
    int num_red = this->get_band_by_wave_length(rgb_default.red);
    int num_green = this->get_band_by_wave_length(rgb_default.green);
    int num_blue = this->get_band_by_wave_length(rgb_default.blue);

    sm->set_slice(bands[num_red]->get_slice());
    sm->set_red(bands[num_red]->get_slice());
    sm->set_green(bands[num_green]->get_slice());
    sm->set_blue(bands[num_blue]->get_slice());
    QImage im = sm->get_rgb(true);
    sm->set_image(im);
    sm->set_RGB();  sm->set_title("");
    sm->create_icon();
    sm->set_channel_num(0);
    sm->set_brightness(default_brightness);
}

int SpectralImage::append_index(QVector<QVector<double> > slice)
{
//    ***
    slice_magic *sm = new slice_magic();  indexes.append(sm);  sm->set_data_file_name(fname);
    sm->set_slice(slice);
    sm->set_slice_size(height,width);
    sm->set_brightness(default_brightness);
    sm->set_channel_num(indexes.count()-1);
    QImage im = sm->get_index_rgb(true,true);
    sm->set_image(im);
    int i_num = sm->get_channel_num();
    if (i_num > 0) sm->calculateHistogram(true);
    sm->set_Index();
    sm->create_icon();
    return depth + indexes.count() - 1;
}

void SpectralImage::append_mask(slice_magic *sm)
{
    masks.append(sm);  sm->set_data_file_name(fname);
    sm->set_MASK();
    current_mask_index = masks.count() - 1;
}

void SpectralImage::calculateHistogram(bool full, uint16_t num)
{
    QVector<QVector<double> > slice = get_band(num);
    J09::histogramType hg = histogram[num];  // статистика гистограммы

    int h = slice.count();  int w = slice.at(0).count();
    if (full) {
      hg.max = INT_MIN;  hg.min = INT_MAX;

      for(int y = 0; y < h; y++) {
        hg.max = std::max(hg.max, *std::max_element(slice[y].begin(), slice[y].end()));
        hg.min = std::min(hg.min, *std::min_element(slice[y].begin(), slice[y].end()));
      }  // for
      if (hg.max == INT_MIN || hg.min == INT_MAX) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      hg.lower = hg.min;  hg.upper = hg.max;
      hg.sum = .0;  hg.sum_of_part = 100.;
      hg.vx.resize(hg.hcount);  hg.vy.resize(hg.hcount);  hg.vy.fill(0.);
      if (std::abs(hg.max - hg.min) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT - all the values are the same; min/max:" << hg.min << hg.max;
        return;
      }  // error

      double k = (hg.hcount-1.)/(hg.max-hg.min);
      if (std::abs(k) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      for (int i=0; i<hg.hcount; i++) hg.vx[i]=hg.min+i/k;
      for (int y=0; y<h; y++)
          for (int x=0; x<w; x++){
              int num_int = qRound((slice[y][x]-hg.min)*k);
              if (num_int < 0) num_int = 0;
              if (num_int > hg.hcount - 1) num_int = hg.hcount - 1;
              hg.vy[num_int]++;
              hg.sum++;
          }  // for
      histogram[num] = hg;
      return;
    }  // if (full)
    hg.sum_of_part = 0.;
    for(int i=0;i<hg.hcount-1;i++) {
        if (hg.vx[i] < hg.lower) continue;
        if (hg.vx[i] > hg.upper) continue;
        hg.sum_of_part += hg.vy[i];
    }  // for
    if (qAbs(hg.sum) < .000001) {
        hg.sum_of_part = .0;  return;
    }  // if
    hg.sum_of_part = 100.*hg.sum_of_part/hg.sum;

    histogram[num] = hg;
}

QString SpectralImage::get_y_axis_title_for_plot(bool short_title)
{
    QString result("?????");
    if (short_title)
        switch (datatype) {
        case dataType::dtBase : result = "к-т отражения";  break;
        case dataType::dtRGB : result = "яркость";  break;
        case dataType::dtFX10e : result = "яркость";  break;
        }  // switch
    else
        switch (datatype) {
        case dataType::dtBase : result = "коэффициент отражения ( r )";  break;
        case dataType::dtRGB : result = "яркость ( r )";  break;
        case dataType::dtFX10e : result = "яркость ( r )";  break;
        }  // switch

    return result;
}

QImage SpectralImage::get_additional_image(int num)
{
    if (num < 0 || num > indexes.count() - 1) return QImage();
    return indexes[num]->get_image();
}

double SpectralImage::get_current_brightness()
{
    if ( current_slice < 0 || current_slice > depth + indexes.count() - 1) return 3.0;
    if ( current_slice < depth ) return bands[current_slice]->get_brightness();
    return indexes[current_slice - depth]->get_brightness();
}

QImage SpectralImage::get_current_image()
{
    if ( current_slice < 0 || current_slice > depth + indexes.count() - 1) return QImage();
    if ( current_slice < depth ) return bands[current_slice]->get_image();
    return indexes[current_slice - depth]->get_image();
}

QImage SpectralImage::get_current_mask_image()
{
    if ( current_mask_index < 0 || current_mask_index > masks.count() - 1) return QImage();
    return masks[current_mask_index]->get_image();
}

void SpectralImage::set_formula_for_index(double r, QString i_title, QString i_formula)
{
    indexes[current_slice - depth]->h.rotation = r;
    indexes[current_slice - depth]->set_title(i_title);
    indexes[current_slice - depth]->set_formula(i_formula);
}


void SpectralImage::set_current_brightness(double value)
{
    if ( current_slice < 0 || current_slice > depth + indexes.count() - 1) return;
    if ( current_slice < depth ) {
        bands[current_slice]->set_brightness(value);
        return;
    }
    indexes[current_slice - depth]->set_brightness(value);
}

int SpectralImage::set_current_mask_index(int num)
{
    if (num < 0 || num > masks.count() - 1) return -1;
    current_mask_index = num;
    return current_mask_index;
}

slice_magic *SpectralImage::get_current_mask()
{
    return masks.at(current_mask_index);
}

QImage SpectralImage::create_current_mask_image()
{
    slice_magic *sm = get_current_mask();
    QImage mask_img(slice_size, QImage::Format_Mono);
    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++) {
            mask_img.setPixel(x, y, sm->get_mask()[y][x]);
        }  // for

    return mask_img;

}

QImage SpectralImage::get_mask_image(int num)
{
    return get_mask(num)->get_image();
}

QImage SpectralImage::create_mask_image(slice_magic *sm)
{
    QImage mask_img(slice_size, QImage::Format_Mono);
    for(int y = 0; y < slice_size.height(); y++)
        for(int x = 0; x < slice_size.width(); x++) {
            mask_img.setPixel(x, y, sm->get_mask()[y][x]);
        }  // for

    return mask_img;
}

J09::maskRecordType SpectralImage::convert_to_record_from_mask(int num)
{
    slice_magic *sm = get_mask(num);
    J09::maskRecordType mr;
    mr.checked = sm->get_check_state();
    mr.title = sm->get_title();  // наименование
    mr.formula = sm->get_formula();  // общая формула алгоритма
    mr.invers = false;  // инверсное изображение
    mr.formula_step_by_step = sm->get_formula_step();  // общая формула алгоритма разбитая на составляющие
    mr.mask = sm->get_mask();  // маска
    mr.img = sm->get_image();
    mr.brightness = sm->get_brightness();
    mr.rotation = sm->get_rotation();
    return mr;
}

void SpectralImage::delete_all_masks()
{
    foreach(slice_magic *m, masks) delete m;
    masks.clear();
}

void SpectralImage::delete_all_indexes()
{
    for(int i=1; i<indexes.count(); i++) delete indexes[i];
    slice_magic *sm = indexes[0];
    indexes.clear();
    indexes.append(sm);
}

void SpectralImage::delete_checked_indexes()
{
    QList<slice_magic *> ql;    ql.append(indexes[0]);
    for(int i=1; i<indexes.count(); i++)
        if (indexes[i]->get_lw_item()->checkState() == Qt::Checked) delete indexes[i];
        else ql.append(indexes[i]);
    indexes.clear();
    foreach(slice_magic *sm, ql) indexes.append(sm);
}

void SpectralImage::save_checked_indexes(QString indexfilename)
{
    if (indexfilename.isEmpty()) return;
    // определяем количество отмеченных индексов
    int count = 0;
    for(int i=1; i<indexes.count(); i++)
        if (get_index(i)->get_lw_item()->checkState() == Qt::Checked) count++;
    if (!count) return;

    QFile datfile(indexfilename);
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );

    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);

    datstream << count;

    for(int i=1; i<indexes.count(); i++)
        if (get_index(i)->get_lw_item()->checkState() == Qt::Checked) get_index(i)->save(datstream);

    datfile.close();
}

int SpectralImage::load_indexes_from_file(QString indexfilename, QListWidget *ilw)
{
    if (indexfilename.isEmpty()) return 0;

    QFile datfile(indexfilename);
    datfile.open(QIODevice::ReadOnly);
    QDataStream datstream( &datfile );
    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);
    int ilw_count = ilw->count();
    int count;
    datstream >> count;
    if (count == 0) return 0;
    for(int i=0; i<count; i++) {
        slice_magic *sm = new slice_magic();
        indexes.append(sm);  sm->load(datstream);
        QListWidgetItem *lwItem = new QListWidgetItem();
        ilw->addItem(lwItem);
        set_LW_item(lwItem, indexes.count()-1+depth);
    }
    datfile.close();

    return depth + ilw_count;
}

void SpectralImage::save_list_checked_bands(QString lstfilename)
{
    if (lstfilename.isEmpty()) return;
    QFile datfile(lstfilename);
    datfile.open(QIODevice::WriteOnly);
    QTextStream txtstream( &datfile );
    txtstream.setCodec("UTF-8");
    for(int i=0; i<depth; i++)
        if (bands[i]->get_lw_item()->checkState() == Qt::Checked)
            txtstream << i << '\n';
    datfile.close();
}

int SpectralImage::load_list_checked_bands(QString lstfilename)
{
    if (lstfilename.isEmpty()) return 0;
    QList<int> ch_list;
    QFile datfile(lstfilename);
    datfile.open(QIODevice::ReadOnly);
    QTextStream txtstream( &datfile );
    txtstream.setCodec("UTF-8");
    while (!txtstream.atEnd()) {
        QString rl = txtstream.readLine();
        ch_list.append(rl.toInt());
      }
    datfile.close();
    if (ch_list.isEmpty()) return 0;
    // unchecked ALL
    for(int i=0; i<depth; i++) {
        bands[i]->get_lw_item()->setCheckState(Qt::Unchecked);
        bands[i]->get_lw_item()->setHidden(true);
    }
    // checked only for reading from file
    foreach(int num, ch_list) {
        bands[num]->get_lw_item()->setCheckState(Qt::Checked);
        bands[num]->get_lw_item()->setHidden(false);
    }
    return ch_list[0];
}

void SpectralImage::save_checked_indexes_separately_img(QString png)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, indexes) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        if (!sm->get_Index()) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + "." + png;
        QMatrix rm;    rm.rotate(sm->h.rotation);
        QImage img = sm->get_image().transformed(rm);
        if(png == "png") img.save(fn, "png");
        if(png == "jpg") img.save(fn, "jpg");
        if(png == "jpeg") img.save(fn, "jpeg");
    }  // if
}

void SpectralImage::save_checked_indexes_separately_csv()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, indexes) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + ".csv";
        sm->save_index_to_CSV_file(fn);
    }  // if
}

QString SpectralImage::replace_fimpossible_symbol(QString fn)
{  // \ / : * ? "" < > |       \ / : * ? "" < > |
    return fn.replace('\\','_').replace('/','_').replace(':','_').replace('*','_')
            .replace('?','_').replace('""','_').replace('<','_').replace('>','_').replace('|','_');
}

void SpectralImage::save_checked_bands_separately_img(QString png)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, bands) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + "." + png;
        QMatrix rm;    rm.rotate(sm->h.rotation);
        QImage img = sm->get_image().transformed(rm);
        if(png == "png") img.save(fn, "png");
        if(png == "jpg") img.save(fn, "jpg");
        if(png == "jpeg") img.save(fn, "jpeg");
    }  // if
}

void SpectralImage::save_checked_bands_separately_csv()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, bands) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + ".csv";
        sm->save_band_to_CSV_file(fn);
    }  // if
}

void SpectralImage::save_checked_masks_separately_img(QString png)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, masks) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + "." + png;
        QMatrix rm;    rm.rotate(sm->h.rotation);
        QImage img = sm->get_image().transformed(rm);
        if(png == "png") img.save(fn, "png");
        if(png == "jpg") img.save(fn, "jpg");
        if(png == "jpeg") img.save(fn, "jpeg");
    }  // if
}

void SpectralImage::save_checked_masks_separately_csv()
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFileInfo info(get_file_name());
    QString base_fname = writableLocation + "/" + info.completeBaseName() + "/";
    foreach(slice_magic *sm, masks) {
        if(sm->get_lw_item()->checkState() != Qt::Checked) continue;
        QString m_title = replace_fimpossible_symbol(sm->get_title());
        QString fn = base_fname + m_title + ".csv";
        sm->save_mask_to_CSV_file(fn);
    }  // if
}

void SpectralImage::save_indexes_for_restore(QString hidden_dir)
{
    QFile datfile( hidden_dir + "index.index" );
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );

    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);

    datstream << indexes.count();

    for(int i=0; i<indexes.count(); i++) {
        if (indexes[i]->get_lw_item()->checkState() == Qt::Checked) indexes[i]->set_check_state(true);
        else indexes[i]->set_check_state(false);
        indexes[i]->save(datstream);
    }  // for

    datfile.close();
}

void SpectralImage::save_view_param_for_restore(QString hidden_dir)
{
    QFile datfile( hidden_dir + "view.bin" );
    datfile.open(QIODevice::WriteOnly);
    QDataStream datstream( &datfile );

    datstream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    datstream.setByteOrder(QDataStream::LittleEndian);

    datstream << recordView.GlobalScale << recordView.GlobalRotate << recordView.GlobalHScrollBar
              << recordView.GlobalVScrollBar << recordView.GlobalChannelNum << recordView.GlobalChannelStep
              << recordView.GlobalMaskNum << recordView.GlobalViewMode;

    datfile.close();
}

void SpectralImage::update_sheck_visible(int trigger)
{
    QVector<slice_magic *> *vsm;
    switch (trigger) {
    case 1 : vsm = &bands;  // каналы
        break;
    case 2 : vsm = &indexes;  // RGB, индексные изображения
        break;
    case 3 : vsm = &masks;  // маски
        break;
    }  // switch
    for(int i=0; i<vsm->count(); i++) {
        vsm->at(i)->set_check_state(vsm->at(i)->get_lw_item()->checkState() == Qt::Checked);
        vsm->at(i)->set_visible(!vsm->at(i)->get_lw_item()->isHidden());
    }  // for
}


void SpectralImage::set_LW_item(QListWidgetItem *lwItem, int num)
{
    lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
    lwItem->setCheckState(Qt::Unchecked);
    slice_magic *sm;
    if (num < depth) sm = bands[num];  // каналы
    else sm = indexes[num - depth];  // RGB, индексные изображения
    sm->set_LW_item(lwItem);

    if (num == depth) {
        QFont font = lwItem->font();  font.setItalic(true);  font.setBold(true);
        lwItem->setFont(font);
// попытка выделить RGB из списка индексов
        lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
        lwItem->setCheckState(Qt::PartiallyChecked);
    }  // if
}

slice_magic::slice_magic(QObject *parent) {}

void slice_magic::set_title(QString a_title)
{
    if (index) {
        title = a_title;  return;
    }
    if (!rgb && !is_mask ) {
        title = QString("номер %1 * %2 нм").arg(ch_num, 3).arg(wave_length, 7, 'f', 2);
        return;
    }
    if (rgb) {
        title = QString("R:%1нм G:%2 нм B:%3нм").arg(rgb_wl.red).arg(rgb_wl.green).arg(rgb_wl.blue);
        return;
    }
    if (is_mask) title = a_title;
}

void slice_magic::calculateHistogram(bool full)
{
//    QVector<QVector<double> > slice = get_band(num);
//    J09::histogramType h = h;  // статистика гистограммы

    int height = slice.count();  int width = slice.at(0).count();
    if (full) {
      h.max = INT_MIN;  h.min = INT_MAX;

      for(int y = 0; y < height; y++) {
        h.max = std::max(h.max, *std::max_element(slice[y].begin(), slice[y].end()));
        h.min = std::min(h.min, *std::min_element(slice[y].begin(), slice[y].end()));
      }  // for
      if (h.max == INT_MIN || h.min == INT_MAX) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      h.lower = h.min;  h.upper = h.max;
      h.sum = .0;  h.sum_of_part = 100.;
      h.vx.resize(h.hcount);  h.vy.resize(h.hcount);  h.vy.fill(0.);
      if (std::abs(h.max - h.min) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT - all the values are the same; min/max:" << h.min << h.max;
        return;
      }  // error

      double k = (h.hcount-1.)/(h.max-h.min);
      if (std::abs(k) < 0.000001) {
        qDebug() << "slice IS NOT CORRECT!!!";
        return;
      }  // error

      for (int i=0; i<h.hcount; i++) h.vx[i]=h.min+i/k;
      for (int y=0; y<height; y++)
          for (int x=0; x<width; x++){
              int num_int = qRound((slice[y][x]-h.min)*k);
              if (num_int < 0) num_int = 0;
              if (num_int > h.hcount - 1) num_int = h.hcount - 1;
              h.vy[num_int]++;
              h.sum++;
          }  // for
      return;
    }  // if (full)
    h.sum_of_part = 0.;
    for(int i=0;i<h.hcount-1;i++) {
        if (h.vx[i] < h.lower) continue;
        if (h.vx[i] > h.upper) continue;
        h.sum_of_part += h.vy[i];
    }  // for
    if (qAbs(h.sum) < .000001) {
        h.sum_of_part = .0;  return;
    }  // if
    h.sum_of_part = 100.*h.sum_of_part/h.sum;
}

QImage slice_magic::get_rgb(bool enhance_contrast) {

  if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();

  if(rgb_b.isEmpty() || rgb_g.isEmpty() || rgb_r.isEmpty()) {
    qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE RGB IMAGE";
    return QImage();
  }
  if(rgb_b.count() != slice_size.height() || rgb_g.count() != slice_size.height() || rgb_r.count() != slice_size.height()) {
    qDebug() << "CRITICAL - WRONG SLICE HEIGHT";
    return QImage();
  }
  if(rgb_b[0].count() != slice_size.width() || rgb_g[0].count() != slice_size.width() || rgb_r[0].count() != slice_size.width()) {
    qDebug() << "CRITICAL - WRONG SLICE WIDTH";
    return QImage();
  }

  double max_r = INT_MIN, max_g = INT_MIN, max_b = INT_MIN,
         min_r = INT_MAX, min_g = INT_MAX, min_b = INT_MAX;
  for(int y = 0; y < slice_size.height(); y++) {
    max_r = std::max(max_r, *std::max_element(rgb_r[y].begin(), rgb_r[y].end()));
    max_g = std::max(max_g, *std::max_element(rgb_g[y].begin(), rgb_g[y].end()));
    max_b = std::max(max_b, *std::max_element(rgb_b[y].begin(), rgb_b[y].end()));
    min_r = std::min(min_r, *std::min_element(rgb_r[y].begin(), rgb_r[y].end()));
    min_g = std::min(min_g, *std::min_element(rgb_g[y].begin(), rgb_g[y].end()));
    min_b = std::min(min_b, *std::min_element(rgb_b[y].begin(), rgb_b[y].end()));
  }
  QImage img(slice_size, QImage::Format_RGB32);
  for(int y = 0; y < slice_size.height(); y++) {
    QRgb *im_scLine = reinterpret_cast<QRgb *>(img.scanLine(y));
    for(int x = 0; x < slice_size.width(); x++) {
      if(enhance_contrast) {
        im_scLine[x] = qRgb(qRound((rgb_r[y][x] - min_r) / (max_r - min_r) * 255.0),
                            qRound((rgb_g[y][x] - min_g) / (max_g - min_g) * 255.0),
                            qRound((rgb_b[y][x] - min_b) / (max_b - min_b) * 255.0));
      } else im_scLine[x] = qRgb(qRound(rgb_r[y][x] * 255.0), qRound(rgb_g[y][x] * 255.0), qRound(rgb_b[y][x] * 255.0));
    }
  }
  return img;
}

QImage slice_magic::get_index_rgb(bool enhance_contrast, bool colorized)
{
    if(slice_size.width() <= 0 || slice_size.height() <= 0) return QImage();

    if(slice.isEmpty()) {
      qDebug() << "CRITICAL! AN EMPTY SLICE RETRIEVED WHILE CONSTRUCTING THE INDEX IMAGE";
      return QImage();
    }
    if(slice.count() != slice_size.height() || slice[0].count() != slice_size.width()) {
      qDebug() << "CRITICAL! - WRONG SLICE HEIGHT OR WIDTH";
      return QImage();
    }
    double max_bw = INT_MIN, min_bw = INT_MAX;
    for(int y = 0; y < slice_size.height(); y++) {
      max_bw = std::max(max_bw, *std::max_element(slice[y].begin(), slice[y].end()));
      min_bw = std::min(min_bw, *std::min_element(slice[y].begin(), slice[y].end()));
    }

    // NDVI -0.621685 0.915984

    QImage index_img(slice_size, QImage::Format_RGB32);

    if (colorized)  // с использованием цветовой схемы
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                double virtual_wlen = h.wl380 + (slice[y][x] - min_bw)
                        * (h.wl781 - h.wl380) / (max_bw - min_bw);
                im_scLine[x] = get_rainbow_RGB(virtual_wlen);
            }  // for int x
        }  // for int y
    else  // черно-белое изображение
        for(int y = 0; y < slice_size.height(); y++) {
            QRgb *im_scLine = reinterpret_cast<QRgb *>(index_img.scanLine(y));
            for(int x = 0; x < slice_size.width(); x++) {
                if(enhance_contrast) {
                    double d = (slice[y][x] - min_bw) / (max_bw - min_bw) * 255.0;
                    im_scLine[x] =  qRgb(qRound(d), qRound(d), qRound(d));
                } else {
                    double dd = slice[y][x] * 255.0;
                    im_scLine[x] = qRgb(qRound(dd), qRound(dd), qRound(dd));
                }  // if
            }    // for int x
        }  // for int y

    return index_img;
}

void slice_magic::create_icon()
{
    if (rgb) { icon = QIcon(":/icons/palette.png"); return; }
    if (index) { icon = QIcon(":/icons/rainbow_icon_cartoon_style.png"); return; }
    if (wave_length < 781) {  // видимый диапазон
        QColor color(get_rainbow_RGB(wave_length));
        QPixmap pixmap(16,16);
        pixmap.fill(color);
        QIcon tmp_ico(pixmap);
        icon = tmp_ico;
    } else
        icon = QIcon(":/icons/color_NIR_32.png");
}

void slice_magic::convert_to_mask_from_record(J09::maskRecordType mr)
{
    set_check_state(mr.checked);
    set_title(mr.title);  // наименование
    set_formula(mr.formula);  // общая формула алгоритма
    set_inverse(mr.invers);  // инверсное изображение
    set_formula_step(mr.formula_step_by_step);  // общая формула алгоритма разбитая на составляющие
    set_mask(mr.mask);  // маска
    set_image(mr.img);
    set_brightness(mr.brightness);
    set_rotation(mr.rotation);
}

void slice_magic::save_band_to_CSV_file(QString data_file_name)
{
    if (!band) return;
    QStringList csv_list;
    QFileInfo fi(fname);
    csv_list.append(QString("Файл;;%1").arg(fi.completeBaseName()));
    csv_list.append(QString("Номер канала;;%1").arg(ch_num));
    csv_list.append(QString("Длина волны;;%1").arg(wave_length, 7, 'f', 2).replace('.', ','));

    for(int y = 0; y < slice_size.height(); y++) {
        QString str;
        for(int x = 0; x < slice_size.width(); x++)
            if (x == slice_size.width() - 1) str.append(QString("%1").arg(slice[y][x]));
            else str.append(QString("%1;").arg(slice[y][x]));
        csv_list.append(str.replace('.', ','));
    }  // for
    QFile file(data_file_name);
    file.remove();
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    foreach(QString str, csv_list) stream << str << endl;
    stream.flush();
    file.close();
}

void slice_magic::save_index_to_CSV_file(QString data_file_name)
{
    if (!index) return;
    QStringList csv_list;
    QFileInfo fi(fname);
    csv_list.append(QString("Файл;;;;%1").arg(fi.completeBaseName()));
    csv_list.append(QString("Наименование индекса;;;;%1").arg(get_title()));
    csv_list.append(QString("Формула индекса;;;;%1").arg(get_formula()));
    QString max_min = QString("%1;%2").arg(h.max).arg(h.min).replace('.', ',');
    csv_list.append(QString("Масимум\\минимум индекса;;;;%1").arg(max_min));

    for(int y = 0; y < slice_size.height(); y++) {
        QString str;
        for(int x = 0; x < slice_size.width(); x++)
            if (x == slice_size.width() - 1) str.append(QString("%1").arg(slice[y][x]));
            else str.append(QString("%1;").arg(slice[y][x]));
        csv_list.append(str.replace('.', ','));
    }  // for
    QFile file(data_file_name);
    file.remove();
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    foreach(QString str, csv_list) stream << str << endl;
    stream.flush();
    file.close();
 }

void slice_magic::save_mask_to_CSV_file_from_slice(QString data_file_name, bool inv)
{
    if (!index) return;
    QStringList csv_list;
    QFileInfo fi(fname);
    csv_list.append(QString("Файл;;;;%1").arg(fi.completeBaseName()));
    csv_list.append(QString("Наименование маски;;;;%1").arg(get_title()));
    csv_list.append(QString("Формула маски;;;;%1").arg(get_formula()));

    for(int y = 0; y < slice_size.height(); y++) {
        QString str;
        for(int x = 0; x < slice_size.width(); x++) {
            QString s0("0"), s1("1");
            if (inv) { s0 = "1";  s1 = "0"; }
            if (slice[y][x] >= h.lower && slice[y][x] <= h.upper) str.append(s1);
            else str.append(s0);
            if (x < slice_size.width() - 1) str.append(";");
        }  // for
        csv_list.append(str.replace('.', ','));
    }  // for

    QFile file(data_file_name);
    file.remove();
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    foreach(QString str, csv_list) stream << str << endl;
    stream.flush();
    file.close();
}

void slice_magic::save_mask_to_CSV_file(QString data_file_name)
{
    if (!is_mask) return;
    QStringList csv_list;
    QFileInfo fi(fname);
    csv_list.append(QString("Файл;;;;%1").arg(fi.completeBaseName()));
    csv_list.append(QString("Наименование маски;;;;%1").arg(get_title()));
    QStringList gfs = get_formula_step();
    for (int i=0; i<gfs.count(); i++)
        if (i==0) csv_list.append(QString("Формула маски;;;;%1").arg(gfs[i]));
        else csv_list.append(QString(";;;;%1").arg(gfs[i]));

    for(int y = 0; y < slice_size.height(); y++) {
        QString str;
        for(int x = 0; x < slice_size.width(); x++)
            if (x == slice_size.width() - 1) str.append(QString("%1").arg(mask[y][x]));
            else str.append(QString("%1;").arg(mask[y][x]));
        csv_list.append(str.replace('.', ','));
    }  // for
    QFile file(data_file_name);
    file.remove();
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    foreach(QString str, csv_list) stream << str << endl;
    stream.flush();
    file.close();
}

QRgb slice_magic::get_rainbow_RGB(double Wavelength)
{
    static double Gamma = 0.80;
    static double IntensityMax = 255;

    double factor;
    double Red,Green,Blue;

    if((Wavelength >= 380) && (Wavelength<440)){
        Red = -(Wavelength - 440) / (440 - 380);
        Green = 0.0;
        Blue = 1.0;
    }else if((Wavelength >= 440) && (Wavelength<490)){
        Red = 0.0;
        Green = (Wavelength - 440) / (490 - 440);
        Blue = 1.0;
    }else if((Wavelength >= 490) && (Wavelength<510)){
        Red = 0.0;
        Green = 1.0;
        Blue = -(Wavelength - 510) / (510 - 490);
    }else if((Wavelength >= 510) && (Wavelength<580)){
        Red = (Wavelength - 510) / (580 - 510);
        Green = 1.0;
        Blue = 0.0;
    }else if((Wavelength >= 580) && (Wavelength<645)){
        Red = 1.0;
        Green = -(Wavelength - 645) / (645 - 580);
        Blue = 0.0;
    }else if((Wavelength >= 645) && (Wavelength<781)){
        Red = 1.0;
        Green = 0.0;
        Blue = 0.0;
    }else{
        Red = 0.0;
        Green = 0.0;
        Blue = 0.0;
    }

// Let the intensity fall off near the vision limits

    if((Wavelength >= 380) && (Wavelength<420)){
        factor = 0.3 + 0.7*(Wavelength - 380) / (420 - 380);
    }else if((Wavelength >= 420) && (Wavelength<701)){
        factor = 1.0;
    }else if((Wavelength >= 701) && (Wavelength<781)){
        factor = 0.3 + 0.7*(780 - Wavelength)  / (780 - 700);
    }else{
        factor = 0.0;
    }

    int r = 0, g = 0, b = 0;

// Don't want 0^x = 1 for x <> 0
    const double zero = 0.00000001;
    if (Red < zero) r = 0;
    else r = qRound(IntensityMax * qPow(Red * factor, Gamma));
    r = qMin(255, r);
    if (Green < zero) g = 0;
    else g = qRound(IntensityMax * qPow(Green * factor, Gamma));
    g = qMin(255, g);
    if (Blue < zero) b = 0;
    else b = qRound(IntensityMax * qPow(Blue * factor, Gamma));
    b = qMin(255, b);

    return qRgb(r,g,b);
}

void slice_magic::save(QDataStream &stream)
{
    stream << check_state << visible;
    stream << title << wave_length << ch_num << brightness << rotation;
    stream << slice << rgb_r << rgb_g << rgb_b << mask << image << icon;                         // иконка для списка выбора
    stream << band << rgb << index << is_mask << formula << formula_step_by_step
           << inverse  << rgb_wl.red << rgb_wl.green << rgb_wl.blue
           << slice_size << fname;

    stream << h.min << h.max << h.sum << h.hcount << h.vx << h.vy
           << h.lower << h.upper << h.sum_of_part << h.wl380 << h.wl781
           << h.brightness << h.___plotmouse << h.___sbrightness
           << h.imgPreview << h.colorized << h.rotation << h.rgb_preview;
}

void slice_magic::load(QDataStream &stream)
{
    stream >> check_state >> visible;
    stream >> title >> wave_length >> ch_num >> brightness >> rotation;
    stream >> slice >> rgb_r >> rgb_g >> rgb_b >> mask >> image >> icon;                         // иконка для списка выбора
    stream >> band >> rgb >> index >> is_mask >> formula >> formula_step_by_step
           >> inverse  >> rgb_wl.red >> rgb_wl.green >> rgb_wl.blue
           >> slice_size >> fname;

    stream >> h.min >> h.max >> h.sum >> h.hcount >> h.vx >> h.vy
           >> h.lower >> h.upper >> h.sum_of_part >> h.wl380 >> h.wl781
           >> h.brightness >> h.___plotmouse >> h.___sbrightness
           >> h.imgPreview >> h.colorized >> h.rotation >> h.rgb_preview;
}
