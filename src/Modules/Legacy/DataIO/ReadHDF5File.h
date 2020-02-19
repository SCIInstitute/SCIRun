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


///
/// @file  ReadHDF5File.h
///
/// @author
///    Allen Sanderson
///    SCI Institute
///    University of Utah
/// @date   May 2003
///

#if !defined(ReadHDF5File_h)
#define ReadHDF5File_h

#include <Dataflow/Network/Module.h>
#include <Core/Util/TypeDescription.h>

#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

#define MAX_PORTS 8
#define MAX_DIMS 6

class ReadHDF5File : public Module {
protected:
  enum { MERGE_NONE=0,   MERGE_LIKE=1,   MERGE_TIME=2 };

public:
  ReadHDF5File(GuiContext *context);

  virtual ~ReadHDF5File();

  virtual void execute();

  void ReadandSendData( string& filename,
                        vector< string >& pathList,
                        vector< string >& datasethList,
                        int which );

  void parseDatasets( string datasets,
                      vector<string>& pathhList,
                      vector<string>& datasethList );

  unsigned int parseTimeSeriesDatasets( vector<string>& pathhList,
                                        vector<string>& datasethList,
                                        vector< vector<string> >& frame_paths,
                                        vector< vector<string> >&
                                          frame_datasets );

  string getTimeRoot( vector<string>& nameList,
                      vector<string>& times,
                      std::string::size_type& d0,
                      std::string::size_type& d1 );

  vector<int> getDatasetDims( string filename, string group, string dataset );

  //  float* readGrid( string filename );
  //  float* readData( string filename );
  NrrdDataHandle readDataset( string filename, string path, string dataset );

  string getDumpFileName( string filename );
  bool checkDumpFile( string filename, string dumpname );
  int createDumpFile( string filename, string dumpname );

  virtual void tcl_command(GuiArgs&, void*);

protected:
  void getTtimeSeriesSlice( string new_filename,
                            vector< vector<string> >& frame_paths,
                            vector< vector<string> >& frame_datasets );


  int increment(int which, int lower, int upper);

  bool is_mergeable(NrrdDataHandle h1, NrrdDataHandle h2);

  void joinNrrds( vector<NrrdDataHandle> &vec, unsigned int merge_type );

protected:
  GuiInt         gui_have_hdf5_;
  GuiInt         gui_power_app_;
  GuiString      gui_power_app_cmd_;

  GuiFilename    gui_filename_;
  GuiString      gui_datasets_;
  GuiString      gui_dumpname_;
  GuiString      gui_ports_;

  GuiInt         gui_ndims_;

  GuiInt         gui_merge_data_;
  GuiInt         gui_assume_svt_;
  GuiInt         gui_time_series_;

  GuiString      gui_time_series_gui_frame_;
  GuiString      gui_time_series_tab_;
  GuiString      gui_basic_tab_;
  GuiString      gui_extended_tab_;
  GuiString      gui_playmode_tab_;

  GuiInt         gui_selectable_min_;
  GuiInt         gui_selectable_max_;
  GuiInt         gui_selectable_inc_;
  GuiInt         gui_range_min_;
  GuiInt         gui_range_max_;

  GuiString      gui_playmode_;
  GuiInt         gui_current_;
  GuiString      gui_execmode_;
  GuiInt         gui_delay_;
  GuiInt         gui_inc_amount_;
  //update_type_ must be declared after selectable_max_ which is
  //traced in the tcl code. If update_type_ is set to Auto having it
  //last will prevent the net from executing when it is instantiated.
  GuiString      gui_update_type_;

  int            inc_;

  GuiInt gui_have_groups_;
  GuiInt gui_have_attributes_;
  GuiInt gui_have_datasets_;

  GuiInt gui_continuous_;

  GuiString gui_selectionString_;
  GuiInt    gui_regexp_;
  GuiString gui_allow_selection_;
  GuiInt    gui_read_error_;

  GuiInt    gui_max_dims_;

  vector< GuiInt* > gui_dims_;
  vector< GuiInt* > gui_starts_;
  vector< GuiString* > gui_starts2_;
  vector< GuiInt* > gui_counts_;
  vector< GuiString* > gui_counts2_;
  vector< GuiInt* > gui_strides_;
  vector< GuiString* > gui_strides2_;

  string old_filename_;
  string old_datasets_;
  time_t old_filemodification_;

  string sel_filename_;
  string sel_datasets_;
  time_t sel_filemodification_;

  bool loop_;
};


} // end namespace SCIRun

#endif // ReadHDF5File_h
