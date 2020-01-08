/*
 * qml_wrapper.cpp -- QML wrapper
 * Copyright (c) 2019 Akhil K Gangadharan <helloimakhil@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <QImage>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QString>
#include <QTimer>
#include <QEventLoop>

#include <framework/mlt_log.h>
#include "qml_wrapper.h"
#include "common.h"
#include "qmlrenderer.h"

void debugz(char debug_message[] )
{
      qDebug() << "====== MLT CALL =========";
      qDebug() << debug_message;
      qDebug() << "==========================";
}

void renderKdenliveTitle(producer_ktitle_qml self, mlt_frame frame,
                         mlt_image_format format, int width, int height,
                         double position, int force_refresh)
{

      // Obtain the producer
      mlt_producer producer = &self->parent;
      mlt_properties producer_props = MLT_PRODUCER_PROPERTIES(producer);

      mlt_properties properties = MLT_FRAME_PROPERTIES(frame);

      pthread_mutex_lock(&self->mutex);

      self->current_image = NULL;
      mlt_properties_set_data( producer_props, "_cached_image", NULL, 0, NULL, NULL );

      if (!createQApplicationIfNeeded(MLT_PRODUCER_SERVICE(producer)))
      {
            pthread_mutex_unlock(&self->mutex);
            return;
      }

      int image_size = width * height * 4;
      // must be extracted from kdenlive qml title
      self->rgba_image = (uint8_t *)mlt_pool_alloc(image_size);

#if QT_VERSION >= 0x050200
      // QImage::Format_RGBA8888 was added in Qt5.2
      // Initialize the QImage with the MLT image because the data formats
      // match.
      QImage::Format image_format = QImage::Format_RGBA8888;
      QImage img(self->rgba_image, width, height, image_format);
#else
      QImage::Format image_format = QImage::Format_ARGB32;
      QImage img(width, height, image_format);
#endif
      
      //TODO: Write code for no resource scenario,  write code to parse in loadFromQml()
      img.fill(0);

      self->renderer->render(img);

      self->format = mlt_image_rgb24a;
      convert_qimage_to_mlt_rgba(&img, self->rgba_image, width, height);
      self->current_image = (uint8_t *)mlt_pool_alloc(image_size);

      memcpy(self->current_image, self->rgba_image, image_size);

      mlt_properties_set_data(producer_props, "_cached_buffer",
                              self->rgba_image, image_size, mlt_pool_release,
                              NULL);
      mlt_properties_set_data(producer_props, "_cached_image",
                              self->current_image, image_size, mlt_pool_release,
                              NULL);
      self->current_width = width;
      self->current_height = height;
      uint8_t *alpha = NULL;
      if ((alpha = mlt_frame_get_alpha(frame)))
      {
            self->current_alpha = (uint8_t *)mlt_pool_alloc(width * height);
            memcpy(self->current_alpha, alpha, width * height);
            mlt_properties_set_data(producer_props, "_cached_alpha",
                                    self->current_alpha, width * height,
                                    mlt_pool_release, NULL);
      }

      // Convert image to requested format
      if (format != mlt_image_none && format != mlt_image_glsl &&
          format != self->format)
      {
            uint8_t *buffer = NULL;
            if (self->format != mlt_image_rgb24a)
            {
                  // Image buffer was previously converted, revert to original
                  // rgba buffer
                  self->current_image = (uint8_t *)mlt_pool_alloc(image_size);
                  memcpy(self->current_image, self->rgba_image, image_size);
                  mlt_properties_set_data(producer_props, "_cached_image",
                                          self->current_image, image_size,
                                          mlt_pool_release, NULL);
                  self->format = mlt_image_rgb24a;
            }

            // First, set the image so it can be converted when we get it
            mlt_frame_replace_image(frame, self->current_image, self->format,
                                    width, height);
            mlt_frame_set_image(frame, self->current_image, image_size, NULL);
            self->format = format;

            // get_image will do the format conversion
            mlt_frame_get_image(frame, &buffer, &format, &width, &height, 0);

            // cache copies of the image and alpha buffers
            if (buffer)
            {
                  image_size =
                      mlt_image_format_size(format, width, height, NULL);
                  self->current_image = (uint8_t *)mlt_pool_alloc(image_size);
                  memcpy(self->current_image, buffer, image_size);
                  mlt_properties_set_data(producer_props, "_cached_image",
                                          self->current_image, image_size,
                                          mlt_pool_release, NULL);
            }
            if ((buffer = mlt_frame_get_alpha(frame)))
            {
                  self->current_alpha =
                      (uint8_t *)mlt_pool_alloc(width * height);
                  memcpy(self->current_alpha, buffer, width * height);
                  mlt_properties_set_data(producer_props, "_cached_alpha",
                                          self->current_alpha, width * height,
                                          mlt_pool_release, NULL);
            }
      }
      pthread_mutex_unlock(&self->mutex);
      mlt_properties_set_int(properties, "width", self->current_width);
      mlt_properties_set_int(properties, "height", self->current_height);
}
