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

#include <Interface/Modules/Render/MovieRecorder.h>

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTemporaryFile>
#include <QThread>

using namespace SCIRun::Gui;

namespace
{
  /// How long the writer waits for ffmpeg to drain a frame before deciding the
  /// encoder is wedged. Blocking here is deliberate: it back-pressures onto the
  /// bounded queue, which drops frames instead of hanging the GUI.
  constexpr int encoderWriteTimeoutMs = 5000;
  constexpr int encoderFinishTimeoutMs = 30000;

  QString directoryOf(const QString& path)
  {
    return QFileInfo(path).absolutePath();
  }

  /// QFileInfo::isWritable() reports the directory's permission bits, which lie
  /// on network shares and read-only volumes. Actually create a file.
  bool directoryAcceptsWrites(const QString& dir)
  {
    QTemporaryFile probe(dir + "/.scirun-movie-XXXXXX");
    return probe.open();
  }
}

QString MovieSettings::suffix() const
{
  switch (format)
  {
    case MovieFormat::Mp4: return "mp4";
    case MovieFormat::Gif: return "gif";
    case MovieFormat::PngSequence:
    default: return "png";
  }
}

MovieFormat SCIRun::Gui::movieFormatFromSuffix(const QString& suffix)
{
  const auto lower = suffix.toLower();
  if (lower == "mp4")
    return MovieFormat::Mp4;
  if (lower == "gif")
    return MovieFormat::Gif;
  return MovieFormat::PngSequence;
}

//---------------------------------------------------------------------------
// MovieFrameWriter
//---------------------------------------------------------------------------

MovieFrameWriter::~MovieFrameWriter()
{
  closeEncoder();
}

void MovieFrameWriter::begin(const MovieSettings& settings, const QSize& frameSize)
{
  settings_ = settings;
  frameSize_ = frameSize;
  frames_ = 0;
  active_ = true;

  if (settings_.needsEncoder())
  {
    if (!startEncoder())
      return;
  }
  else
  {
    const QFileInfo info(settings_.outputPath);
    frameNameTemplate_ = info.absolutePath() % "/" % info.completeBaseName() % "_%1." % settings_.suffix();
  }
}

bool MovieFrameWriter::startEncoder()
{
  const auto ffmpeg = MovieRecorder::ffmpegPath();
  if (ffmpeg.isEmpty())
  {
    fail("ffmpeg was not found on the system path; choose the PNG frame sequence format instead.");
    return false;
  }

  QStringList args;
  args << "-y"
       << "-f" << "rawvideo"
       << "-pixel_format" << "rgba"
       << "-video_size" << QString("%1x%2").arg(frameSize_.width()).arg(frameSize_.height())
       << "-framerate" << QString::number(settings_.frameRate)
       << "-i" << "-";

  if (settings_.format == MovieFormat::Mp4)
  {
    // h264 needs even dimensions; the ViewScene is whatever size the user left it.
    args << "-vf" << "scale=trunc(iw/2)*2:trunc(ih/2)*2"
         << "-c:v" << "libx264"
         << "-pix_fmt" << "yuv420p"
         << "-crf" << "18"
         << "-movflags" << "+faststart";
  }
  else
  {
    // Single-pass palette generation, otherwise GIFs come out badly dithered.
    args << "-filter_complex" << "[0:v] split [a][b];[a] palettegen [p];[b][p] paletteuse";
  }
  args << settings_.outputPath;

  encoder_ = new QProcess;
  encoder_->setProcessChannelMode(QProcess::MergedChannels);
  encoder_->start(ffmpeg, args);
  if (!encoder_->waitForStarted(10000))
  {
    fail("Could not start ffmpeg: " % encoder_->errorString());
    closeEncoder();
    return false;
  }
  return true;
}

void MovieFrameWriter::writeFrame(const QImage& frame)
{
  if (queued_)
    --*queued_;
  if (!active_)
    return;

  // A resize mid-recording would otherwise corrupt the stream, and ffmpeg was
  // told the frame size up front.
  auto image = frame.size() == frameSize_ ? frame : frame.scaled(frameSize_,
    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  if (encoder_)
  {
    if (encoder_->state() != QProcess::Running)
    {
      fail("ffmpeg exited early: " % QString::fromUtf8(encoder_->readAll()));
      return;
    }
    image = image.convertToFormat(QImage::Format_RGBA8888);
    const auto rowBytes = static_cast<qint64>(image.width()) * 4;
    for (int y = 0; y < image.height(); ++y)
    {
      if (encoder_->write(reinterpret_cast<const char*>(image.constScanLine(y)), rowBytes) != rowBytes)
      {
        fail("Failed writing frame data to ffmpeg: " % encoder_->errorString());
        return;
      }
    }
    while (encoder_->bytesToWrite() > 0)
    {
      if (!encoder_->waitForBytesWritten(encoderWriteTimeoutMs))
      {
        fail("ffmpeg stopped accepting frames: " % encoder_->errorString());
        return;
      }
    }
  }
  else
  {
    const auto name = frameNameTemplate_.arg(frames_ + 1, 5, 10, QChar('0'));
    if (!image.save(name))
    {
      fail("Failed to write frame " % name);
      return;
    }
  }

  ++frames_;
  Q_EMIT frameWritten(frames_);
}

void MovieFrameWriter::end()
{
  if (!active_)
    return;
  active_ = false;

  const auto path = settings_.outputPath;
  const auto frames = frames_;

  if (encoder_)
  {
    encoder_->closeWriteChannel();
    if (!encoder_->waitForFinished(encoderFinishTimeoutMs))
      encoder_->kill();
    const auto status = encoder_->exitStatus();
    const auto code = encoder_->exitCode();
    const auto output = QString::fromUtf8(encoder_->readAll());
    closeEncoder();
    if (status != QProcess::NormalExit || code != 0)
    {
      Q_EMIT failed("ffmpeg failed to finish the movie.\n\n" % output.right(2000));
      return;
    }
  }

  Q_EMIT finished(path, frames);
}

void MovieFrameWriter::closeEncoder()
{
  if (!encoder_)
    return;
  if (encoder_->state() != QProcess::NotRunning)
  {
    encoder_->closeWriteChannel();
    if (!encoder_->waitForFinished(2000))
      encoder_->kill();
  }
  delete encoder_;
  encoder_ = nullptr;
}

void MovieFrameWriter::fail(const QString& message)
{
  active_ = false;
  closeEncoder();
  Q_EMIT failed(message);
}

//---------------------------------------------------------------------------
// MovieRecorder
//---------------------------------------------------------------------------

MovieRecorder::MovieRecorder(QObject* parent) : QObject(parent)
{
  qRegisterMetaType<MovieSettings>("SCIRun::Gui::MovieSettings");

  writerThread_ = new QThread(this);
  writer_ = new MovieFrameWriter(&queued_);
  writer_->moveToThread(writerThread_);

  connect(writerThread_, &QThread::finished, writer_, &QObject::deleteLater);
  connect(this, &MovieRecorder::beginRequested, writer_, &MovieFrameWriter::begin);
  connect(this, &MovieRecorder::frameRequested, writer_, &MovieFrameWriter::writeFrame);
  connect(this, &MovieRecorder::endRequested, writer_, &MovieFrameWriter::end);
  connect(writer_, &MovieFrameWriter::frameWritten, this, &MovieRecorder::onFrameWritten);
  connect(writer_, &MovieFrameWriter::failed, this, &MovieRecorder::onFailed);
  connect(writer_, &MovieFrameWriter::finished, this, &MovieRecorder::onFinished);

  writerThread_->start();
}

MovieRecorder::~MovieRecorder()
{
  if (recording_)
    stop();
  writerThread_->quit();
  writerThread_->wait();
}

QString MovieRecorder::ffmpegPath()
{
  static const QString path = QStandardPaths::findExecutable("ffmpeg");
  return path;
}

QString MovieRecorder::validate(const MovieSettings& settings)
{
  if (settings.outputPath.trimmed().isEmpty())
    return "No output file has been chosen.";

  if (settings.frameRate < 1)
    return "The frame rate must be at least 1 frame per second.";

  const auto dir = directoryOf(settings.outputPath);
  if (!QDir().mkpath(dir))
    return "Could not create the output directory:\n" % dir;

  if (!directoryAcceptsWrites(dir))
    return "The output directory is not writable:\n" % dir;

  const QFileInfo out(settings.outputPath);
  if (out.exists() && !out.isWritable())
    return "The output file is not writable:\n" % settings.outputPath;

  if (settings.needsEncoder() && !ffmpegAvailable())
    return "ffmpeg was not found on the system path. Install ffmpeg, or record a PNG frame sequence instead.";

  return {};
}

bool MovieRecorder::start(const MovieSettings& settings, const QSize& frameSize)
{
  if (recording_)
    return true;

  const auto problem = validate(settings);
  if (!problem.isEmpty())
  {
    Q_EMIT recordingFailed(problem);
    return false;
  }
  if (frameSize.isEmpty())
  {
    Q_EMIT recordingFailed("The scene has no size to record yet.");
    return false;
  }

  framesWritten_ = 0;
  framesDropped_ = 0;
  queued_ = 0;
  recording_ = true;
  Q_EMIT beginRequested(settings, frameSize);
  return true;
}

void MovieRecorder::addFrame(const QImage& frame)
{
  if (!recording_)
    return;

  if (queued_.load() >= maxQueuedFrames_)
  {
    ++framesDropped_;
    return;
  }

  ++queued_;
  // Detach from the framebuffer grab: the image crosses a thread boundary.
  Q_EMIT frameRequested(frame.copy());
}

void MovieRecorder::stop()
{
  if (!recording_)
    return;
  recording_ = false;
  Q_EMIT endRequested();
}

void MovieRecorder::onFrameWritten(int totalFrames)
{
  framesWritten_ = totalFrames;
  Q_EMIT frameCountChanged(totalFrames);
}

void MovieRecorder::onFailed(const QString& message)
{
  recording_ = false;
  queued_ = 0;
  Q_EMIT recordingFailed(message);
}

void MovieRecorder::onFinished(const QString& outputPath, int totalFrames)
{
  recording_ = false;
  queued_ = 0;
  Q_EMIT recordingFinished(outputPath, totalFrames);
}
