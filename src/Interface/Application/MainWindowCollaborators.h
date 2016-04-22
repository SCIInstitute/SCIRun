/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef INTERFACE_APPLICATION_MAINWINDOWCOLLABORATORS_H
#define INTERFACE_APPLICATION_MAINWINDOWCOLLABORATORS_H

#ifndef Q_MOC_RUN
#include <Core/Logging/LoggerInterface.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Singleton.h>
#include <set>
#include <Interface/Application/NetworkEditor.h>  //TODO
#endif
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QMutex>

class QTextEdit;
class QTreeWidget;
class QComboBox;
class QStatusBar;

namespace SCIRun {
namespace Gui {

  class TextEditAppender : public Core::Logging::LegacyLoggerInterface, public Core::Logging::LogAppenderStrategy
  {
  public:
    explicit TextEditAppender(QTextEdit* text, bool regressionMode = false) :
      text_(text), regressionMode_(regressionMode) {}

    void log(const QString& message) const;

    virtual void error(const std::string& msg) const override;
    virtual void warning(const std::string& msg) const override;
    virtual void remark(const std::string& msg) const override;
    virtual void status(const std::string& msg) const override;

    virtual void log4(const std::string& message) const override;
  private:
    QTextEdit* text_;
    mutable QMutex mutex_;
    bool regressionMode_;
  };

  class TreeViewModuleGetter : public CurrentModuleSelection
  {
  public:
    explicit TreeViewModuleGetter(QTreeWidget& tree) : tree_(tree) {}
    virtual QString text() const override;
    virtual QString clipboardXML() const override;
    virtual bool isModule() const override;
    virtual bool isClipboardXML() const override;
  private:
    QTreeWidget& tree_;
  };

  class ComboBoxDefaultNotePositionGetter : public DefaultNotePositionGetter
  {
  public:
    explicit ComboBoxDefaultNotePositionGetter(QComboBox& combo) : combo_(combo) {}
    virtual NotePosition position() const override;
  private:
    QComboBox& combo_;
  };

  typedef boost::variant<QAction*, QWidget*> InputWidget;

  // During network execution, this class manages disabling parts of the GUI
  class WidgetDisablingService : public QObject, boost::noncopyable
  {
    Q_OBJECT

    CORE_SINGLETON( WidgetDisablingService );

  private:
    WidgetDisablingService() {}
  public Q_SLOTS:
    void disableInputWidgets();
    void enableInputWidgets();
    void temporarilyDisableService();
    void temporarilyEnableService();
  public:
    void addNetworkEditor(NetworkEditor* ne);
    void addWidget(const InputWidget& w);
    void removeWidget(const InputWidget& w);

    template <class Iter>
    void addWidgets(Iter begin, Iter end)
    {
      std::copy(begin, end, std::back_inserter(inputWidgets_));
    }
  private:
    NetworkEditor* ne_ {nullptr};
    std::vector<InputWidget> inputWidgets_;
    bool serviceEnabled_ {true};
  };

  inline void addWidgetToExecutionDisableList(const InputWidget& w)
  {
    WidgetDisablingService::Instance().addWidget(w);
  }
  inline void removeWidgetFromExecutionDisableList(const InputWidget& w)
  {
    WidgetDisablingService::Instance().removeWidget(w);
  }


  class FileDownloader : public QObject
  {
    Q_OBJECT

  public:
    explicit FileDownloader(QUrl imageUrl, QStatusBar* statusBar, QObject *parent = 0);
    QByteArray downloadedData() const { return downloadedData_; }

  Q_SIGNALS:
    void downloaded();

  private Q_SLOTS:
    void fileDownloaded(QNetworkReply* reply);
    void downloadProgress(qint64 received, qint64 total);
  private:
    QNetworkAccessManager webCtrl_;
    QNetworkReply* reply_;
    QByteArray downloadedData_;
    QStatusBar* statusBar_;
  };

  class ToolkitDownloader : public QObject
  {
    Q_OBJECT
  public:
    explicit ToolkitDownloader(QObject* infoObject, QStatusBar* statusBar, QWidget* parent = 0);
  private Q_SLOTS:
    void showMessageBox();
    void saveToolkit();

  private:
    void downloadIcon(); //TODO: cache somehow
    FileDownloader* iconDownloader_;
    FileDownloader* zipDownloader_;
    QString iconUrl_, fileUrl_, filename_;
    QDir toolkitDir_;
    QStatusBar* statusBar_;
  };

}
}
#endif
