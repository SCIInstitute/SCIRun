/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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



/*
 *  ReadNrrd.cc: Read in a Nrrd
 *
 *  Written by:
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *   February 2001
 *
 */

#include <Modules/Legacy/Teem/DataIO/ReadNrrd.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>
//#include <Core/Util/sci_system.h>

//#include <sys/stat.h>
#include <Core/ImportExport/GenericIEPlugin.h>

//#ifdef _WIN32
//#include <process.h> // for getpid
//#endif

using namespace SCIRun;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Dataflow::Networks;
using namespace Core::Algorithms;

#if 0
namespace SCITeem {

using namespace SCIRun;

class ReadNrrd : public Module {
public:
  ReadNrrd(SCIRun::GuiContext* ctx);
  virtual ~ReadNrrd();
  virtual void execute();

private:
  bool read_nrrd();
  bool read_file(const std::string& filename);
  bool write_tmpfile(const std::string& filename, std::string *tmpfilename, const std::string& conv_command);

  GuiString       types_;
  GuiString       filetype_;
  GuiFilename     filename_;
  GuiString       from_env_;

  NrrdDataHandle  read_handle_;

  std::string     old_filename_;
  time_t          old_filemodification_;
  int             cached_label_generation_;
  char *          cached_label_;
};

} // end namespace SCITeem

using namespace SCITeem;

DECLARE_MAKER(ReadNrrd)
#endif

const ModuleLookupInfo ReadNrrd::staticInfo_("ReadNrrd", "DataIO", "Teem");

ReadNrrd::ReadNrrd() :
  Module(staticInfo_)
//  types_(get_ctx()->subVar("types", false)),
  // filetype_(get_ctx()->subVar("filetype")),
  // filename_(get_ctx()->subVar("filename"), ""),
  // from_env_(get_ctx()->subVar("from-env"),""),
  // read_handle_(0),
  // old_filemodification_(0),
  // cached_label_generation_(0),
  // cached_label_(0)
{
  INITIALIZE_PORT(Output_Data);
}

std::string ReadNrrd::fileTypeList()
{
   /*
  NrrdIEPluginManager mgr;
  std::vector<std::string> importers;
  mgr.get_importer_list(importers);

  std::string importtypes = "{";
  importtypes += "{{Nrrd Files}    {.nhdr .nrrd .png .txt .vtk} } ";
  importtypes += "{{VFF}           {.vff} } "; // temporary
  importtypes += "{{PICT}          {.pic pict} } "; // temporary
  importtypes += "{{Vista File}    {.v} } "; // temporary
  importtypes += "{{NrrdData File} {.nd} } ";
  importtypes += "{{Nrrd File Any} {.*} } ";

  importtypes += "}";

  types_.set(importtypes);
  */
  //return ".nrrd";//TODO
  NrrdIEPluginManager mgr;
  return makeGuiTypesListForImport(mgr);
}

// TODO: read_nrrd and read_files need to be rewritten with better return value strategies
// Also, file status should be in a helper function (FileUtils?)
//
// Return true if handle_ was changed, otherwise return false.  This
// return value does not necessarily signal an error!
NrrdDataHandle ReadNrrd::read_nrrd()
{
  NrrdDataHandle nrrd;
  const auto filename = get_state()->getValue(Variables::Filename).toFilename();
  const auto filenameStr = filename.string();
  if (filenameStr.empty())
  {
    error("Please specify nrrd filename");
    return nullptr;
  }

#if 0
  // Read the status of this file so we can compare modification timestamps.
  struct stat buf;
  if (stat(fn.c_str(), &buf) == - 1)
  {
    error(std::string("NrrdReader error - file not found: '")+fn+"'");
    return false;
  }

  // If we haven't read yet, or if it's a new filename,
  //  or if the datestamp has changed -- then read...
  time_t new_filemodification = buf.st_mtime;

  if(!read_handle_.get_rep() ||
     fn != old_filename_ ||
     new_filemodification != old_filemodification_)
#endif
  {
    #if 0
    old_filemodification_ = new_filemodification;
    old_filename_=fn;
    read_handle_ = 0;
    #endif

    const std::string ext(".nd");
    const std::string vff_ext(".vff");
    const std::string vff_conv_command("vff2nrrd %f %t");
    const std::string pic_ext(".pic");
    const std::string pic_ext2(".pict");
    const std::string pic_conv_command("PictToNrrd %f %t");
    const std::string vista_ext(".v");
    const std::string vista_conv_command("VistaToNrrd %f %t");

    // check that the last 3 chars are .nd for us to pio
    if (filename.extension() == ext)
    {
      auto stream = auto_istream(filenameStr, getLogger());
      if (!stream)
      {
        error("Error reading file '" + filenameStr + "'.");
        return nullptr;
      }

      // Read the file
      Pio(*stream, nrrd);
      if (!nrrd || stream->error())
      {
        error("Error reading data from file '" + filenameStr + "'.");
        return nullptr;
      }
    }
    else
    { // assume it is just a nrrd
      if (filename.extension() == vff_ext)
      {
        std::string tmpfilename;
        write_tmpfile(filenameStr, &tmpfilename, vff_conv_command);
        return read_file(tmpfilename);
      }
      if (filename.extension() == pic_ext || filename.extension() == pic_ext2)
      {
        std::string tmpfilename;
        write_tmpfile(filenameStr, &tmpfilename, pic_conv_command);
        return read_file(tmpfilename);
      }
      if (filename.extension() == vista_ext)
      {
        std::string tmpfilename;
        write_tmpfile(filenameStr, &tmpfilename, vista_conv_command);
        return read_file(tmpfilename);
      }
      return read_file(filenameStr);
    }
    return nrrd;
  }
}

NrrdDataHandle
ReadNrrd::read_file(const std::string& fn)
{
  // Restrict TEEM access: it is not thread safe
  NrrdGuard g;
  NrrdDataHandle n(new NrrdData);
  if (0 != nrrdLoad(n->getNrrd(), airStrdup(fn.c_str()), nullptr))
  {
    // Ugly error handling
    char *err = biffGetDone(NRRD);
    error("Read error on '" + fn + "': " + err);
    free(err);
    return nullptr;
  }
  return n;
}

bool
ReadNrrd::write_tmpfile(const std::string& filename, std::string* tmpfilename,
                          const std::string& conv_command)
{
  return false;
  #if 0
  std::string::size_type loc = filename.find_last_of("/");
  const std::string basefilename =
    (loc==std::string::npos)?filename:filename.substr(loc+1);

  // Base filename with first extension removed.
  loc = basefilename.find_last_of(".");
  const std::string basenoext = basefilename.substr(0, loc);

  // Filename with first extension removed.
  loc = filename.find_last_of(".");
  const std::string noext = filename.substr(0, loc);

  // Temporary filename.
  //tmpfilename = "/tmp/" + basenoext + "-" +
  //  to_string((unsigned int)(getpid())) + ".nhdr";
  std::string tmpdir = std::string(sci_getenv("SCIRUN_TMP_DIR")) + "/";
  tmpfilename->assign(tmpdir + basenoext + "-" +
		      to_string((unsigned int)(getpid())) + ".nhdr");

  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  std::string command =
    std::string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    conv_command;
  while ((loc = command.find("%f")) != std::string::npos)
  {
    command.replace(loc, 2, '"'+filename+'"');
  }
  while ((loc = command.find("%t")) != std::string::npos)
  {
    command.replace(loc, 2, '"'+*tmpfilename+'"');
  }
  const int status = sci_system(command.c_str());
  if (status)
  {
    remark("'" + command + "' failed.  Read may not work.");
  }
  return true;
  #endif
}

void
ReadNrrd::execute()
{
  update_state(NeedData);

  if (needToExecute())
  {
    auto nrrd = read_nrrd();

    if (!nrrd)
    {
      error("Please load a nrrd.");
      return;
    }

#if 0
    // A hack to make PowerApps at least work with old types of Nrrds
    Nrrd* nrrd = read_handle_->nrrd_;
    if (nrrd->spaceDim == 0)
    {
      size_t dim = nrrd->dim;
      for (size_t j=0; j<dim;j++)
      {
        if (IsNan(nrrd->axis[j].min) && IsNan(nrrd->axis[j].max)
          && !IsNan(nrrd->axis[j].spacing))
        {
          nrrd->axis[j].min = 0.0;
          nrrd->axis[j].max = (nrrd->axis[j].size-1)*nrrd->axis[j].spacing;
        }
    }
  }
#endif

    sendOutput(Output_Data, nrrd);

    update_state(Completed);
  }
}
