/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#ifndef INTERFACE_MODULES_MOVIE_RECORDER_H
#define INTERFACE_MODULES_MOVIE_RECORDER_H

#include <atomic>
#include <Interface/qt_include.h>
#include <Interface/Modules/Render/share.h>

class QProcess;
class QThread;

namespace SCIRun
{
  namespace Gui
  {
    enum class MovieFormat
    {
      PngSequence = 0,
      Mp4         = 1,
      Gif         = 2
    };

    struct SCISHARE MovieSettings
    {
      QString outputPath;
      MovieFormat format {MovieFormat::PngSequence};
      int frameRate {30};

      bool needsEncoder() const { return format != MovieFormat::PngSequence; }
      /// Extension the format expects, without the dot. Doubles as the token
      /// the format is saved under in module state.
      QString suffix() const;
    };

    SCISHARE MovieFormat movieFormatFromSuffix(const QString& suffix);

    /// Writes captured frames on a worker thread. Never touched directly by the
    /// dialog: MovieRecorder owns it and talks to it with queued connections.
    class MovieFrameWriter : public QObject
    {
      Q_OBJECT
    public:
      /// queued counts frames handed off but not yet consumed; the writer
      /// decrements it so the producer knows when to start dropping.
      explicit MovieFrameWriter(std::atomic<int>* queued) : queued_(queued) {}
      ~MovieFrameWriter() override;

    public Q_SLOTS:
      void begin(const SCIRun::Gui::MovieSettings& settings, const QSize& frameSize);
      void writeFrame(const QImage& frame);
      void end();

    Q_SIGNALS:
      void frameWritten(int totalFrames);
      void failed(const QString& message);
      void finished(const QString& outputPath, int totalFrames);

    private:
      void fail(const QString& message);
      bool startEncoder();
      void closeEncoder();

      std::atomic<int>* queued_ {nullptr};
      MovieSettings settings_;
      QSize frameSize_;
      QString frameNameTemplate_;   ///< PNG sequence only
      QProcess* encoder_ {nullptr}; ///< ffmpeg, when the format needs one
      int frames_ {0};
      bool active_ {false};
    };

    /// Frame-grab recorder for the ViewScene. Capture happens on the GUI thread
    /// (grabFramebuffer must), encoding on a worker thread. When the writer
    /// falls behind, frames are dropped rather than queued without bound --
    /// SCIRun 4's recorder hung instead (issue #279).
    class SCISHARE MovieRecorder : public QObject
    {
      Q_OBJECT
    public:
      explicit MovieRecorder(QObject* parent = nullptr);
      ~MovieRecorder() override;

      /// Empty when the settings are recordable, otherwise the reason why not.
      /// Called before recording starts so a bad path fails loudly. Creates the
      /// output directory, since "is it writable" is only answerable once it exists.
      static QString validate(const MovieSettings& settings);
      static QString ffmpegPath();
      static bool ffmpegAvailable() { return !ffmpegPath().isEmpty(); }

      bool isRecording() const { return recording_; }
      int framesWritten() const { return framesWritten_; }
      int framesDropped() const { return framesDropped_; }

      /// Returns false and emits recordingFailed() when the output is unusable.
      bool start(const MovieSettings& settings, const QSize& frameSize);
      void addFrame(const QImage& frame);
      void stop();

    Q_SIGNALS:
      void frameCountChanged(int frames);
      void recordingFailed(const QString& message);
      void recordingFinished(const QString& outputPath, int frames);

      // internal, to the writer thread
      void beginRequested(const SCIRun::Gui::MovieSettings& settings, const QSize& frameSize);
      void frameRequested(const QImage& frame);
      void endRequested();

    private Q_SLOTS:
      void onFrameWritten(int totalFrames);
      void onFailed(const QString& message);
      void onFinished(const QString& outputPath, int totalFrames);

    private:
      /// Deep enough to ride out a slow disk or a stalled encoder, small enough
      /// that a wedged writer cannot eat memory.
      static constexpr int maxQueuedFrames_ = 60;

      QThread* writerThread_ {nullptr};
      MovieFrameWriter* writer_ {nullptr};
      bool recording_ {false};
      int framesWritten_ {0};
      int framesDropped_ {0};
      std::atomic<int> queued_ {0};
    };
  }
}

Q_DECLARE_METATYPE(SCIRun::Gui::MovieSettings)

#endif
