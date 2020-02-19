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
/// @file  ReadHDF5File.cc
///
/// @author
///    Allen Sanderson
///    SCI Institute
///    University of Utah
/// @date   May 2003
///


#include <sci_defs/hdf5_defs.h>
#include <sci_defs/stat64_defs.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Thread/Time.h>
#include <Dataflow/Modules/DataIO/ReadHDF5File.h>

#ifdef HAVE_HDF5
#include "hdf5.h"
#include "WriteHDF5DumpFile.h"
#endif

namespace SCIRun {

DECLARE_MAKER(ReadHDF5File)

ReadHDF5File::ReadHDF5File(GuiContext *context)
  : Module("ReadHDF5File", context, Source, "DataIO", "SCIRun"),
#ifdef HAVE_HDF5
    gui_have_hdf5_(context->subVar("have_HDF5"), 1),
#else
    gui_have_hdf5_(context->subVar("have_HDF5"), 0),
#endif
    gui_power_app_(context->subVar("power_app"), 0),
    gui_power_app_cmd_(context->subVar("power_app_commmand", "")),

    gui_filename_(context->subVar("filename"), ""),
    gui_datasets_(context->subVar("datasets"), ""),
    gui_dumpname_(context->subVar("dumpname"), ""),

    gui_ports_(context->subVar("ports"), ""),
    gui_ndims_(context->subVar("ndims"), 0),

    gui_merge_data_(context->subVar("mergeData"), 1),
    gui_assume_svt_(context->subVar("assumeSVT"), 1),
    gui_time_series_(context->subVar("time_series"), 0),

    gui_time_series_gui_frame_(context->subVar("time_series_gui_frame", "")),
    gui_time_series_tab_(context->subVar("time_series_tab"), ""),
    gui_basic_tab_(context->subVar("basic_tab"), ""),
    gui_extended_tab_(context->subVar("extended_tab"), ""),
    gui_playmode_tab_(context->subVar("playmode_tab"), ""),

    gui_selectable_min_(context->subVar("selectable_min"), 0),
    gui_selectable_max_(context->subVar("selectable_max"), 100),
    gui_selectable_inc_(context->subVar("selectable_inc"), 1),
    gui_range_min_(context->subVar("range_min"), 0),
    gui_range_max_(context->subVar("range_max"), 0),

    gui_playmode_(context->subVar("playmode"), "once"),
    gui_current_(context->subVar("current"), 0),
    gui_execmode_(context->subVar("execmode"), "init"),
    gui_delay_(context->subVar("delay"), 0),
    gui_inc_amount_(context->subVar("inc-amount"), 1),
    gui_update_type_(context->subVar("update_type"), "On Release"),
    inc_(1),

    gui_have_groups_(context->subVar("have_group"), 0),
    gui_have_attributes_(context->subVar("have_attributes"), 0),
    gui_have_datasets_(context->subVar("have_datasets"), 0),
    gui_continuous_(context->subVar("continuous"), 0),

    gui_selectionString_(context->subVar("selectionString"), ""),
    gui_regexp_(context->subVar("regexp"), 0),
    gui_allow_selection_(context->subVar("allow_selection"), "true" ),
    gui_read_error_(context->subVar("read_error"), 0),

    gui_max_dims_(context->subVar("max_dims"), MAX_DIMS),

    loop_(false)
{
  for( unsigned int ic=0; ic<MAX_DIMS; ic++ ) {
    char idx[16];

    sprintf( idx, "%d-dim", ic );
    gui_dims_   .push_back(new GuiInt(context->subVar(idx), 1) );
    sprintf( idx, "%d-start", ic );
    gui_starts_ .push_back(new GuiInt(context->subVar(idx), 0) );
    sprintf( idx, "%d-start2", ic );
    gui_starts2_ .push_back(new GuiString(context->subVar(idx), "0") );
    sprintf( idx, "%d-count", ic );
    gui_counts_ .push_back(new GuiInt(context->subVar(idx), 1) );
    sprintf( idx, "%d-count2", ic );
    gui_counts2_ .push_back(new GuiString(context->subVar(idx), "1") );
    sprintf( idx, "%d-stride", ic );
    gui_strides_.push_back(new GuiInt(context->subVar(idx), 1) );
    sprintf( idx, "%d-stride2", ic );
    gui_strides2_.push_back(new GuiString(context->subVar(idx), "1") );
  }
}

ReadHDF5File::~ReadHDF5File() {
}



// Allows nrrds to join along tuple, scalar and vector sets can not be joined,
// or allows for a multiple identical nrrds to assume a time series,
// and be joined along a new time axis.
bool
ReadHDF5File::is_mergeable(NrrdDataHandle h1, NrrdDataHandle h2)
{
  std::string::size_type pos;

  string nrrdName1, nrrdName2, grp1, grp2;

  h1->get_property( "Name", nrrdName1 );
  h2->get_property( "Name", nrrdName2 );

  if (gui_merge_data_.get() == MERGE_LIKE) {
    grp1 = nrrdName1;
    grp2 = nrrdName2;

    pos = grp1.find_last_of("-");
    if( pos != std::string::npos )
      grp1.erase( pos, grp1.length()-pos );

    pos = grp2.find_last_of("-");
    if( pos != std::string::npos )
      grp2.erase( pos, grp2.length()-pos );

    if( grp1 != grp2 )
      return false;
  }

  float time1, time2, misc1, misc2;

  if (gui_merge_data_.get() == MERGE_TIME) {
    h1->get_property( "time", time1 );
    h2->get_property( "time", time2 );

    h1->remove_property( "time" );
    h2->remove_property( "time" );

    h1->get_property( "Step number", misc1 );
    h2->get_property( "Step number", misc2 );

    h1->remove_property( "Step number" );
    h2->remove_property( "Step number" );
  }

  // The names are the only properties that are allowed to be different
  // when merging so remove them before testing the rest of the properties.
  h1->remove_property( "Name" );
  h2->remove_property( "Name" );

  bool pass = true;

  if( (*((PropertyManager *) h1.get_rep()) !=
       *((PropertyManager *) h2.get_rep())) ) pass = false;

  // Restore the names
  h1->set_property( "Name", nrrdName1, false );
  h2->set_property( "Name", nrrdName2, false );

  if (gui_merge_data_.get() == MERGE_TIME) {
    h1->set_property( "time", time1, false );
    h2->set_property( "time", time2, false );

    h1->set_property( "Step number", misc1, false );
    h2->set_property( "Step number", misc2, false );
  }

  if( !pass )
    return false;

  Nrrd* n1 = h1->nrrd_;
  Nrrd* n2 = h2->nrrd_;

  if (n1->type != n2->type)
    return false;

  if (n1->dim  != n2->dim)
    return false;

  // Compare the dimensions.
  for (unsigned int i=0; i<n1->dim; i++) {
    if (n1->axis[i].size != n2->axis[i].size)
      return false;
  }

  return true;
}


void ReadHDF5File::joinNrrds( vector<NrrdDataHandle> &vec,
            unsigned int merge_type )
{
  vector<Nrrd*> join_me;
  vector<NrrdDataHandle>::iterator niter = vec.begin();

  NrrdDataHandle n = *niter;
  ++niter;
  join_me.push_back(n->nrrd_);

  string nrrdName, groupName, dataName;

  n->get_property( "Name", groupName );
  std::string::size_type pos = groupName.find_last_of("-");
  if( pos != std::string::npos )
    groupName.erase( pos, groupName.length()-pos );

  n->get_property( "Name", dataName );
  pos = dataName.find_last_of("-"); // Erase the Group
  if( pos != std::string::npos )
    dataName.erase( 0, pos );
  pos = dataName.find_last_of(":"); // Erase the Kind
  if( pos != std::string::npos )
    dataName.erase( pos, dataName.length()-pos );

  nrrdName = groupName + dataName;

  while (niter != vec.end()) {
    NrrdDataHandle n = *niter;
    ++niter;
    join_me.push_back(n->nrrd_);

    if (merge_type == MERGE_LIKE) {
      n->get_property( "Name", dataName );
      pos = dataName.find_last_of("-"); // Erase the Group
      if( pos != std::string::npos )
        dataName.erase( 0, pos );
      pos = dataName.find_last_of(":"); // Erase the Kind
      if( pos != std::string::npos )
        dataName.erase( pos, dataName.length()-pos );

      nrrdName += dataName;
    }
  }

  NrrdData* onrrd = new NrrdData();

  int axis;
  int incr = 1; // incr.

  if (merge_type == MERGE_LIKE)
    axis = 0;               // axis
  else // if (merge_type == MERGE_TIME)
    axis = join_me[0]->dim; // axis

  onrrd->nrrd_ = nrrdNew();
  if (nrrdJoin(onrrd->nrrd_, &join_me[0], join_me.size(), axis, incr)) {
    char *err = biffGetDone(NRRD);
    error(string("Join Error: ") +  err);
    free(err);
    return;
  }

  // set new kinds for joined nrrds
  if (merge_type == MERGE_LIKE) {
    onrrd->nrrd_->axis[0].label = strdup("Merged Data");

    if (gui_assume_svt_.get() && join_me.size() == 3) {
      onrrd->nrrd_->axis[0].kind = nrrdKind3Vector;
      nrrdName += string(":Vector");
    } else if (gui_assume_svt_.get() && join_me.size() == 6) {
      onrrd->nrrd_->axis[0].kind = nrrdKind3DSymMatrix;
      nrrdName += string(":Matrix");
    } else if (gui_assume_svt_.get() && join_me.size() == 9) {
      onrrd->nrrd_->axis[0].kind = nrrdKind3DMatrix;
      nrrdName += string(":Matrix");
    } else {
      onrrd->nrrd_->axis[0].kind = nrrdKindDomain;
      nrrdName += string(":Scalar");
    }

    for(unsigned int i=1; i<onrrd->nrrd_->dim; i++) {
      onrrd->nrrd_->axis[i].kind = nrrdKindDomain;
      onrrd->nrrd_->axis[i].label = join_me[0]->axis[i].label;
    }

  } else if (merge_type == MERGE_TIME) {

    onrrd->nrrd_->axis[axis].kind = nrrdKindDomain;
    onrrd->nrrd_->axis[axis].label = strdup("Time");

    // remove all numbers from name
    string s(nrrdName);
    nrrdName.clear();

    const string nums("0123456789");

    // test against valid char set.
    for(string::size_type i = 0; i < s.size(); i++) {
      bool in_set = false;
      for (unsigned int c = 0; c < nums.size(); c++) {
        if (s[i] == nums[c]) {
          in_set = true;
          break;
        }
      }

      if (in_set) { nrrdName.push_back('X' ); }
      else        { nrrdName.push_back(s[i]); }
    }
  }

  // Copy the properties.
  NrrdDataHandle handle = NrrdDataHandle(onrrd);

  *((PropertyManager *) handle.get_rep()) =
    *((PropertyManager *) n.get_rep());

  // Take care of the axis label and the nrrd name.
  onrrd->set_property( "Name", nrrdName, false );

  // clear the nrrds;
  vec.clear();
  vec.push_back(handle);
}


void ReadHDF5File::execute() {

  // In the case of a time series the datasets may be separate so
  // allow for a file reader. This only updates the filename - nothing
  // else like the dumpfile or the treeview.
  StringHandle sHandle;

  if (get_input_handle( "Full filename", sHandle, false )) {
    gui_filename_.set( sHandle->get() );
    gui_filename_.reset();
  }

  MatrixHandle mIndexHandle;

  if( gui_time_series_.get() == 1 &&
      get_input_handle( "Current Index", mIndexHandle, false ) )
  {
    // Do nothing - just get the handle so inputs_changed_ can be
    // reset.
  }

  MatrixHandle mSlabHandle;

  if( get_input_handle( "Slab Index", mSlabHandle, false ) )
  {
    // Do nothing - just get the handle so inputs_changed_ can be
    // reset.
  }

  // It does not matter if the handles changes it is the name or the
  // current index change that counts.
  inputs_changed_ = false;

#ifdef HAVE_HDF5
  string filename(gui_filename_.get());
  string datasets(gui_datasets_.get());

  if( filename.length() == 0 ) {
    error( string("No HDF5 file selected.") );
    return;
  }

  if( datasets.length() == 0 ) {
    error( string("No HDF5 datasets selected.") );
    return;
  }

  // Read the status of this file so we can compare modification timestamps
#ifdef HAVE_STAT64
  struct stat64 buf;
  if (stat64(filename.c_str(), &buf) == -1)
#else
  struct stat buf;
  if (stat(filename.c_str(), &buf) == -1)
#endif
  {
    error( string("HDF5 File not found ") + filename );
    return;
  }

  time_t filemodification = buf.st_mtime;

  // If we haven't read yet, or if it's a new filename,
  //  or if the datestamp has changed -- then read...
  if( filename         != old_filename_ ||
      filemodification != old_filemodification_ ||
      datasets         != old_datasets_ ) {

    old_filemodification_ = filemodification;
    old_filename_         = filename;
    old_datasets_         = datasets;

    sel_filemodification_ = filemodification;
    sel_filename_         = filename;
    sel_datasets_         = datasets;

    inputs_changed_ = true;
  }


  if( mSlabHandle.get_rep() ) {

    /// The matrix is optional. If present make sure it is a nx4 matrix.
    /// The row indices is the axis index. The column is the data.
    if( ( mSlabHandle->ncols() != 4) )
    {
      error( "Input slab matrix is not a nx4 matrix" );
      return;
    }

    if( mSlabHandle->nrows() != gui_ndims_.get() ) {
      error( "Input slab matrix does not have the same dimensions as entries." );
      return;
    }

    /// Sanity check. Make sure the gui dimensions match the matrix
    /// dimensions.
    for( int ic=0; ic<mSlabHandle->nrows(); ic++ ) {
      if( gui_dims_[ic]->get() != (int) mSlabHandle->get(ic, 3) )
      {
        ostringstream str;
        str << "The dimensions of the slab matrix do match the entries. "
            << " Expected for the " << ic << " dimension "
            << gui_dims_[ic]->get() << " "
            << " got "
            << mSlabHandle->get(ic, 3);
        error( str.str() );
        return;
      }
    }

    bool update = false;

    /// Update the slab entries.
    for( int ic=0; ic<mSlabHandle->nrows(); ic++ ) {

      if( gui_starts_[ic]->get()  != (int) mSlabHandle->get(ic, 0) ||
          gui_counts_[ic]->get()  != (int) mSlabHandle->get(ic, 1) ||
          gui_strides_[ic]->get() != (int) mSlabHandle->get(ic, 2))
      {
        gui_starts_[ic]->set(  (int) mSlabHandle->get(ic, 0) );
        gui_counts_[ic]->set(  (int) mSlabHandle->get(ic, 1) );
        gui_strides_[ic]->set( (int) mSlabHandle->get(ic, 2) );

        gui_starts_[ic]->reset();
        gui_counts_[ic]->reset();
        gui_strides_[ic]->reset();

        inputs_changed_ = update = true;
      }
    }
  }

  // get all the actual values from gui.
  if( gui_merge_data_.changed( true ) ||
      gui_assume_svt_.changed( true ) ) {
    inputs_changed_ = true;
  }

  for( int ic=0; ic<MAX_DIMS; ic++ ) {

    if( gui_starts_ [ic]->changed( true ) ||
        gui_counts_ [ic]->changed( true ) ||
        gui_strides_[ic]->changed( true ) ) {
      inputs_changed_ = true;
    }

    if( gui_dims_[ic]->changed( true ) ) {
      inputs_changed_ = true;
    }

    if( gui_starts_[ic]->get() +
        (gui_counts_[ic]->get()-1) * gui_strides_[ic]->get() >=
        gui_dims_[ic]->get() ) {
      error( "Data selection exceeds bounds." );
      error( "Decrease the start or count or increase the stride." );
      return;
    }
  }

  vector< string > pathList;
  vector< string > datasetList;

  parseDatasets( datasets, pathList, datasetList );

  if( gui_time_series_.get() == 1 ) {

    vector< vector<string> > time_slice_paths;
    vector< vector<string> > time_slice_datasets;

    // Determine the number of time slices.
    int ntime_slices =
      parseTimeSeriesDatasets( pathList, datasetList,
             time_slice_paths, time_slice_datasets);

    if( gui_selectable_max_.get() != ntime_slices-1 ) {
      gui_selectable_max_.set(ntime_slices-1);
      gui_selectable_max_.reset();
    }

    if( mIndexHandle.get_rep() )
    {
      int which = (int) (mIndexHandle->get(0, 0));

      if( which < 0 || (int) time_slice_paths.size() <= which ) {
        ostringstream str;
        str << "Input index is out of range ";
        str << "0 <= " << which << " <= " << (int) time_slice_paths.size();

        error( str.str() );

        return;
      }

      if( which != gui_current_.get() )
      {
        gui_current_.set(which);
        gui_current_.reset();

        inputs_changed_ = true;
      }

      if( inputs_changed_ ||
          !oport_cached("Output 0 Nrrd") )
      {
        ReadandSendData( filename, time_slice_paths[which],
                         time_slice_datasets[which], which );
      }
    }
    else // Manual time series input
    {
      inputs_changed_ = (inputs_changed_ ||
                         gui_current_.changed( true ) ||
                         !oport_cached("Output 0 Nrrd"));
      getTtimeSeriesSlice( filename, time_slice_paths, time_slice_datasets );
    }

  }

  // Basic input
  else if( inputs_changed_ ||
           !oport_cached("Output 0 Nrrd") )
  {
    ReadandSendData( filename, pathList, datasetList, -1 );
  }

#else
  error( "SCIRun was not compiled with HDF5." );
  error( "To recompile SCIRun with HDF5 do the following:" );
  error( "cd to the directory that scirun was built in (~/SCIRun/bin)" );
  error( "cmake -DHDF5_DIR=<path-to-hdf5> <path-to-source>" );
#endif
}

void ReadHDF5File::ReadandSendData( string& filename,
                                    vector< string >& pathList,
                                    vector< string >& datasetList,
                                    int which )
{
  vector< vector<NrrdDataHandle> > nHandles;
  vector< vector<int> > ids;

  vector< int > ports;

  for( unsigned int ic=0; ic<pathList.size(); ic++ ) {
    ports.push_back( -1 );

    NrrdDataHandle nHandle =
      readDataset( filename, pathList[ic], datasetList[ic]);

    if( nHandle != NULL ) {
      bool inserted = false;
      vector<vector<NrrdDataHandle> >::iterator iter = nHandles.begin();
      vector<vector<int> >::iterator iditer = ids.begin();

      while (iter != nHandles.end()) {

        vector<NrrdDataHandle> &vec = *iter;
        vector<int> &idvec = *iditer;

        ++iter;
        ++iditer;

        if(is_mergeable(nHandle, vec[0])) {
          vec.push_back(nHandle);
          idvec.push_back(ic);
          inserted = true;
          break;
        }
      }

      if (!inserted) {
        vector<NrrdDataHandle> nrrdSet;
        nrrdSet.push_back( nHandle );
        nHandles.push_back( nrrdSet );

        vector<int> idSet;
        idSet.push_back( ic );
        ids.push_back( idSet );
      }
    } else {
      error( "No handle for - " + pathList[ic] + "/" + datasetList[ic] );
      return;
    }
  }

  // Merge the like datatypes together.
  if( gui_merge_data_.get() == MERGE_LIKE ) {

    vector<vector<NrrdDataHandle> >::iterator iter = nHandles.begin();
    while (iter != nHandles.end()) {

      vector<NrrdDataHandle> &vec = *iter;
      ++iter;

      if( vec.size() > 1) {
        if( gui_assume_svt_.get() &&
            vec.size() != 3 && vec.size() != 6 && vec.size() != 9) {
          warning( "Assuming Vector or Matrix data but can not merge into a "
                   "Vector or Matrix because there are not 3, 6, or 9 nrrds "
                   "that are alike." );
          return;
        }
        joinNrrds( vec, MERGE_LIKE );
      }
    }
  } else if( gui_merge_data_.get() == MERGE_TIME ) {

    vector< vector<string> > time_slice_paths;
    vector< vector<string> > time_slice_datasets;

    // Get the number of time slices.
    unsigned int ntime_slices =
      parseTimeSeriesDatasets( pathList, datasetList,
             time_slice_paths, time_slice_datasets);

    vector<vector<NrrdDataHandle> >::iterator iter = nHandles.begin();

    while ( iter != nHandles.end()) {

      vector<NrrdDataHandle> &vec = *iter;
      ++iter;

      if( ntime_slices &&
          vec.size()/ntime_slices > 1 && gui_assume_svt_.get() ) {

        if( vec.size()/ntime_slices % 3 &&
            vec.size()/ntime_slices % 6 &&
            vec.size()/ntime_slices % 9) {
          warning( "Assuming Vector or Matrix data but can not merge into a Vector or Matrix because there are not 3, 6, or 9 nrrds that are alike." );
          continue;

        } else {

          unsigned int size = 0;

          if( vec.size()/ntime_slices % 9 == 0 ) size = 9;
          else if( vec.size()/ntime_slices % 6 == 0 ) size = 6;
          else if( vec.size()/ntime_slices % 3 == 0 ) size = 3;

          vector<NrrdDataHandle> new_vec;

          for( unsigned int i=0; i<vec.size(); i+=size ) {

            vector<NrrdDataHandle> tmp_vec;

            for( unsigned int j=0; j<size; ++j ) {

              string nrrdName;
              vec[i+j]->get_property( "Name", nrrdName );

              tmp_vec.push_back( vec[i+j] );
            }

            joinNrrds( tmp_vec, MERGE_LIKE );

            for( unsigned int j=0; j<tmp_vec.size(); ++j ) {
              new_vec.push_back( tmp_vec[j] );
            }
          }

          vec.resize(new_vec.size());

          for( unsigned int i=0; i<new_vec.size(); ++i ) {
            vec[i] = new_vec[i];
          }
        }
      }

      joinNrrds( vec, MERGE_TIME );
    }
  }

  NrrdDataHandle nrrd_output_handles[MAX_PORTS];

  unsigned int cc = 0;

  for( unsigned int ic=0; ic<nHandles.size(); ic++ ) {

    int size = ids[ic].size() / nHandles[ic].size();

    for( unsigned int jc=0; jc<ids[ic].size(); jc++ ) {
      ports[ids[ic][jc]] = cc + jc/size;
    }

    for( unsigned int jc=0; jc<nHandles[ic].size(); jc++ ) {
      if( cc < MAX_PORTS ) {
        nrrd_output_handles[cc] = nHandles[ic][jc];
        nrrd_output_handles[cc]->set_property("Source", string("HDF5"), false);
      }

      ++cc;
    }
  }

  std::ostringstream portStr;


  // Set the dataset offset so that the port can be on the correct dataset.
  unsigned int ds;

  if( gui_time_series_.get() == 1 )
    ds = pathList.size() * gui_current_.get();
  else
    ds = 0;

  // Load the dataset and port location.
  for( unsigned int ic=0; ic<pathList.size(); ic++ ) {
    portStr << std::setw(1) << " "
            << std::setw(6) << ds+ic << std::setw(1) << " "
            << std::setw(2) << ports[ic];
  }

  // Update the ports in the GUI.
  ostringstream str;
  str << get_id() << " updateSelection {" << portStr.str() << "}";

  TCLInterface::execute(str.str().c_str());

  if( cc > MAX_PORTS )
    warning( "More data than availible ports." );

  for( unsigned int ic=cc; ic<MAX_PORTS; ic++ )
    nrrd_output_handles[ic] = 0;

  for( unsigned int ic=0; ic<cc; ic++ ) {

    string portName = string("Output ") + to_string(ic) + string( " Nrrd" );

    // Send the data downstream
    send_output_handle( portName, nrrd_output_handles[ic] );
  }

  DenseMatrix *selected = new DenseMatrix(1);
  selected->put(0, 0, (double) which);

  MatrixHandle matrix_output_handle = MatrixHandle(selected);

  send_output_handle( "Selected Index", matrix_output_handle );


  DenseMatrix *slab = new DenseMatrix(gui_ndims_.get(), 4);

  matrix_output_handle = MatrixHandle(slab);

  for( int ic=0; ic<gui_ndims_.get(); ic++ ) {
    slab->put(ic, 0, gui_starts_[ic]->get() );
    slab->put(ic, 1, gui_counts_[ic]->get() );
    slab->put(ic, 2, gui_strides_[ic]->get() );
    slab->put(ic, 3, gui_dims_[ic]->get() );
  }

  send_output_handle( "Slab Index", matrix_output_handle );
}


void ReadHDF5File::parseDatasets( string datasets,
                                  vector<string>& pathList,
                                  vector<string>& datasetList )
{
  int open = 0;

  std::string::size_type cc = 0;
  std::string::size_type bb = 0;
  std::string::size_type ee = 0;

  string path, dataset;

  while( cc < datasets.length() ) {

    bb = ee = cc;

    if( datasets[bb] == '{' ) {
      // Multiple datasets.

      open = 1;
      ee = bb + 1;

      while( open && ee < datasets.length() ) {
        ee++;

        if( datasets[ee] == '{' )
          open++;
        else if( datasets[ee] == '}' )
          open--;
      }

      path = datasets.substr( bb+1, ee-bb-1);

      cc = ee + 2;

    } else {
      // Single Dataset
      path = datasets;

      cc = datasets.length();
    }

    // Remove the first space.
    path.erase( 1, 1 );

    open = 0;

    // Replace all of spaces that are not in braces '{}'
    // with a forward slash '/'.
    for( unsigned int i=0; i<path.length(); i++ ) {

      if( path[i] == '{' ) {
        open++;
      } else if( path[i] == '}' ) {
        open--;
      } else if( !open && path[i] == ' ' ) {
        path[i] = '/';
      }
    }

    string::size_type i;

    while( (i = path.find("//")) != string::npos) {
      path.erase(i, 1);
    }

    // If still open there is a brace mismatch
    if( open ) {
      error( "Found a path with mismatched braces - " + path );
      return;
    } else {
      std::string::size_type pos;

      // Remove the braces.
      while( (pos = path.find( "{" )) != std::string::npos )
        path.erase( pos, 1 );

      while( (pos = path.find( "}" )) != std::string::npos )
        path.erase( pos, 1 );

      // Get the dataset name which is after the pos forward slash '/'.
      pos = path.find_last_of( "/" );

      // Get the dataset name.
      string dataset( path.substr( pos+1, path.length()-pos) );

      // Remove the dataset name from the path.
      path.erase( pos, path.length()-pos);

      // Just incase the dataset is at the root.
      if( path.length() == 0 ) path = string( "/" );

      pathList.push_back( path );
      datasetList.push_back( dataset );
    }
  }
}


unsigned int
ReadHDF5File::parseTimeSeriesDatasets( vector<string>& pathList,
                                       vector<string>& datasetList,
                                       vector< vector<string> >& time_slice_paths,
                                       vector< vector<string> >&
                                         time_slice_datasets )
{
  time_slice_paths.clear();
  time_slice_datasets.clear();

  if( pathList.size() == 1 ) {
    time_slice_paths.push_back( pathList );
    time_slice_datasets.push_back( datasetList );

    remark( "Found 1 time step with 1 dataset." );

    return 1;
  }

  vector<string> times;
  std::string::size_type d0, d1;

  vector<string>* baseList;

  string root = getTimeRoot( pathList, times, d0, d1 );

  if( root.length() ) {
    baseList = &pathList;

  } else {
    baseList = &datasetList;

    root = getTimeRoot( datasetList, times, d0, d1 );
  }

  // If this happens both the path and datasets have the same names.
  if( root.length() == 0 ) {
    ostringstream str;
    str << "Can not determine the time stepping. ";
    str << "The path and datasets appear to have the same name.";
    error( str.str() );

    return 0;
  }

  // Sort the datasets by time.
  for( unsigned int j=0; j<times.size(); j++ ) {

    string base = root + times[j];

    vector< string > path_list;
    vector< string > dataset_list;

    for( unsigned int i=0; i<pathList.size(); i++ ) {
      if( (*baseList)[i].find( base ) != string::npos ) {

        path_list.push_back( pathList[i] );
        dataset_list.push_back( datasetList[i] );
      }
    }

    time_slice_paths.push_back( path_list );
    time_slice_datasets.push_back( dataset_list );

    // Make sure the paths are the same.
    if( j > 0 ) {

      // The same number of datasets should be found for each time step.
      if( time_slice_paths[0].size()    != time_slice_paths[j].size() ||
          time_slice_datasets[0].size() != time_slice_datasets[j].size() ) {
        ostringstream str;
        str << "The number of datasets in each time step is not the same.\n";

        str << "Expected " << time_slice_paths[0].size();
        str << " paths, got " << time_slice_paths[j].size();
        str << " for time step " << times[j] << endl;

        str << "Expected " << time_slice_datasets[0].size();
        str << " datasets, got " << time_slice_datasets[j].size();
        str << " for time step " << times[j] << endl;

        error( str.str() );

        return 0;

      } else {

        // Make sure the all paths and datasets are the same except for time.
        for( unsigned int i=0; i<time_slice_paths[0].size(); i++ ) {
          if( // The time step is in the path so the last of each path
             // must be the same along with the dataset names.
             (baseList == &pathList &&
              (time_slice_paths[0][i].substr(d1, time_slice_paths[0][i].length()-d1) !=
               time_slice_paths[j][i].substr(d1, time_slice_paths[0][i].length()-d1) ||
               time_slice_datasets[0][i] != time_slice_datasets[j][i] ) ) ||

             // The time step is in the dataset so the last of each
             // dataset name must be the same along with the paths.
             (baseList == &datasetList &&
              (time_slice_datasets[0][i].substr(d1, time_slice_datasets[0][i].length()-d1) !=
               time_slice_datasets[j][i].substr(d1, time_slice_datasets[0][i].length()-d1) ||
               time_slice_paths[0][i] != time_slice_paths[j][i] ) ) ) {

            error( "Animation path and/or dataset mismatch" );
            error( time_slice_paths[0][i] + " " + time_slice_datasets[0][i] );
            error( time_slice_paths[j][i] + " " + time_slice_datasets[j][i] );

            return 0;
          }
        }
      }
    }
  }

  ostringstream str;
  str << "Found " << times.size() << " time steps. ";
  str << "Each time step contains " << time_slice_paths[0].size() << " datasets.";
  remark( str.str() );

  return times.size();
}


string ReadHDF5File::getTimeRoot( vector<string>& nameList,
            vector<string>& times,
            std::string::size_type& d0,
            std::string::size_type& d1 ) {
  string comp = nameList[0];

  for( unsigned int i=1; i<nameList.size(); i++ ) {

    unsigned int len = nameList[i].length();

    // Reduce the size of the comparison to the smallest string.
    if( comp.length() > len )
      comp.replace( len, comp.length() - len, "" );

    // Mark the characters that are different.
    for( unsigned int c=0; c<comp.length(); c++ ) {
      if( comp[c] != nameList[i][c] )
        comp[c] = '?';
    }
  }

  // Find the first difference.
  d0 = comp.find_first_of("?");

  if( d0 == string::npos )
    return string( "" );

  // Find the last difference.
  d1 = comp.find_last_of("?") + 1;

  // Get all of the times.
  for( unsigned int i=0; i<nameList.size(); i++ ) {
    string time = nameList[i].substr( d0, d1-d0 );

    unsigned int j;

    for( j=0; j<times.size(); j++ ) {
      if( time == times[j] )
        break;
    }

    if( j == times.size() )
      times.push_back( time );
  }

  std::sort( times.begin(), times.end() );

  return nameList[0].substr( 0, d0 );
}


vector<int> ReadHDF5File::getDatasetDims( string filename,
                                          string group,
                                          string dataset ) {
  vector< int > idims;

#ifdef HAVE_HDF5
  herr_t status = 0;

  /* Open the file using default properties. */
  hid_t file_id, ds_id, g_id, file_space_id;

  if( (file_id = H5Fopen(filename.c_str(),
       H5F_ACC_RDONLY, H5P_DEFAULT)) < 0 ) {
    error( "Error opening file - " + filename);
    return idims;
  }

  /* Open the group in the file. */

  if( (g_id = H5Gopen(file_id, group.c_str(), H5P_DEFAULT)) < 0 ) {
    error( "Error opening group - " + group );
    return idims;
  }

  /* Open the dataset in the file. */
  if( (ds_id = H5Dopen(g_id, dataset.c_str(), H5P_DEFAULT)) < 0 ) {
    error( "Error opening dataset - " + dataset );
    return idims;
  }

  /* Open the coordinate space in the file. */
  if( (file_space_id = H5Dget_space( ds_id )) < 0 ) {
    error( "Error getting file space. " );
    return idims;
  }

  /* Get the rank (number of dims) in the space. */
  int ndims = H5Sget_simple_extent_ndims(file_space_id);

  if (H5Sis_simple(file_space_id)) {
    if (ndims == 0) {
      /* scalar dataspace */
      idims.push_back( 1 );

    } else {
      /* simple dataspace */
      hsize_t *dims = new hsize_t[ndims];

      /* Get the dims in the space. */
      int ndim = H5Sget_simple_extent_dims(file_space_id, dims, NULL);

      if( ndim != ndims ) {
        error( "Data dimensions not match. " );
        return idims;
      }

      for( int ic=0; ic<ndims; ic++ )
        idims.push_back( dims[ic] );

      delete[] dims;
    }
  }

  /* Terminate access to the data space. */
  if( (status = H5Sclose(file_space_id)) < 0 ) {
    error( "Error closing file space. " );
  }
  /* Terminate access to the dataset. */
  if( (status = H5Dclose(ds_id)) < 0 ) {
    error( "Error closing data set. " );
  }
  /* Terminate access to the group. */
  if( (status = H5Gclose(g_id)) < 0 ) {
    error( "Error closing group. " );
  }
  /* Terminate access to the group. */
  if( (status = H5Fclose(file_id)) < 0 ) {
    error( "Error closing file. " );
  }

#endif

  return idims;
}

#ifdef HAVE_HDF5

static string HDF5Attribute_error_msg;

herr_t add_attribute(hid_t group_id, const char * aname,
         const H5A_info_t *ainfo, void* op_data) {

  herr_t status = 0;

  NrrdData * nrrd = (NrrdData *) (op_data);

  // Do not dulplicate properties.
  if( nrrd->is_property( aname ) )
  {
    return status;
  }

  hid_t attr_id = H5Aopen_name(group_id, aname);

  if (attr_id < 0) {
    HDF5Attribute_error_msg =
      string("Unable to open attribute \"") + aname + "\"";
    status = -1;
  } else {

    hid_t type_id = H5Aget_type( attr_id );
    hid_t file_space_id = H5Aget_space( attr_id );

    if( file_space_id < 0 ) {
      HDF5Attribute_error_msg = "Unable to open data ";
      return -1;
    }

    hid_t mem_type_id;

    switch (H5Tget_class(type_id)) {
    case H5T_STRING:
      // String
      if(H5Tis_variable_str(type_id)) {
        mem_type_id = H5Tcopy(H5T_C_S1);
        H5Tset_size(mem_type_id, H5T_VARIABLE);
      } else {
        mem_type_id = H5Tcopy(type_id);
        H5Tset_cset(mem_type_id, H5T_CSET_ASCII);
      }

      break;

    case H5T_INTEGER:
      // Integer
      mem_type_id = H5T_NATIVE_INT;
      break;

    case H5T_FLOAT:
      if (H5Tequal(type_id, H5T_IEEE_F32BE) ||
          H5Tequal(type_id, H5T_IEEE_F32LE) ||
          H5Tequal(type_id, H5T_NATIVE_FLOAT)) {
        // Float
        mem_type_id = H5T_NATIVE_FLOAT;

      } else if (H5Tequal(type_id, H5T_IEEE_F64BE) ||
                 H5Tequal(type_id, H5T_IEEE_F64LE) ||
                 H5Tequal(type_id, H5T_NATIVE_DOUBLE) ||
                 H5Tequal(type_id, H5T_NATIVE_LDOUBLE)) {
        // Double
        mem_type_id = H5T_NATIVE_DOUBLE;

      } else {
        HDF5Attribute_error_msg = "Undefined HDF5 float";
        return -1;
      }

      break;
    case H5T_REFERENCE:
      return status;
      break;
    default:
      HDF5Attribute_error_msg = "Unknown or unsupported HDF5 data type";
      return -1;
    }

    /* Get the rank (number of dims) in the space. */
    int ndims = H5Sget_simple_extent_ndims(file_space_id);

    int cc = 1;

    {
      hsize_t *dims = new hsize_t[ndims];

      /* Get the dims in the space. */
      int ndim = H5Sget_simple_extent_dims(file_space_id, dims, NULL);

      if( ndim != ndims ) {
        HDF5Attribute_error_msg = "Data dimensions not match.";
        return -1;
      }

      for( int ic=0; ic<ndims; ic++ )
        cc *= dims[ic];

      delete[] dims;
    }

    int size;

    if( H5Tget_size(type_id) > H5Tget_size(mem_type_id) )
      size = cc * H5Tget_size(type_id);
    else
      size = cc * H5Tget_size(mem_type_id);

    char *data = new char[size+1];

    if( data == NULL ) {
      HDF5Attribute_error_msg = "Can not allocate enough memory for the data";
      return -1;
    }

    status = H5Aread(attr_id, mem_type_id, data);

    if( status < 0 ) {
      HDF5Attribute_error_msg = "Can not read data";
      return status;
    }

    // Force a sentinel NULL.
    data[size] = '\0';

    NrrdData * nrrd = (NrrdData *) (op_data);

    if( cc == 1 ) {
      if (mem_type_id == H5T_INTEGER) {
        // Int
        nrrd->set_property( aname, ((int*) data)[0], false );
      } else if (mem_type_id == H5T_NATIVE_FLOAT) {
        // Float
        nrrd->set_property( aname, ((float*) data)[0], false );
      } else if (mem_type_id == H5T_NATIVE_DOUBLE) {
        // Double
        nrrd->set_property( aname, ((double*) data)[0], false );
      } else if(H5Tget_class(type_id) == H5T_STRING ) {
        // String
        if(H5Tis_variable_str(type_id))
          nrrd->set_property( aname, ((char*) data)[0], false );
        else
          nrrd->set_property( aname, string((char*) data), false );
      }
    } else {
      ostringstream str;

      for( int ic=0; ic<cc; ic++ ) {
        if (mem_type_id == H5T_INTEGER) {
          // Int
          str << ((int*) data)[ic];
        } else if (mem_type_id == H5T_NATIVE_FLOAT) {
          // Float
          str << ((float*) data)[ic];
        } else if (mem_type_id == H5T_NATIVE_DOUBLE) {
          // Double
          str << ((double*) data)[ic];
        } else if(H5Tget_class(type_id) == H5T_STRING ) {
          // String
          if(H5Tis_variable_str(type_id))
            str << ((char*) data)[ic];
          else
            str << "\"" << (char*) data  << "\"";
        }
      }

      nrrd->set_property( aname, str.str(), false );
    }

    H5Tclose(type_id);

    H5Aclose(attr_id);

    delete[] data;
  }

  return status;
}
#endif


NrrdDataHandle ReadHDF5File::readDataset( string filename,
                                          string group,
                                          string dataset ) {
#ifdef HAVE_HDF5
  char *data = NULL;

  herr_t status = 0;

  /* Open the file using default properties. */
  hid_t file_id, g_id, ds_id, file_space_id;

  if( (file_id = H5Fopen(filename.c_str(),
       H5F_ACC_RDONLY, H5P_DEFAULT)) < 0 ) {
    error( "Error opening file - " + filename);
    return NULL;
  }

  /* Open the group in the file. */
  if( (g_id = H5Gopen(file_id, group.c_str(), H5P_DEFAULT)) < 0 ) {
    error( "Error opening group - " + group);
    return NULL;
  }

  /* Open the dataset in the file. */
  if( (ds_id = H5Dopen(g_id, dataset.c_str(), H5P_DEFAULT)) < 0 ) {
    error( "Error opening data space - " + dataset);
    return NULL;
  }

  /* Open the coordinate space in the file. */
  if( (file_space_id = H5Dget_space( ds_id )) < 0 ) {
    error( "Error opening file space. " );
    return NULL;
  }

  hid_t type_id = H5Dget_type(ds_id);

  hid_t mem_type_id;

  unsigned int nrrd_type;

  switch (H5Tget_class(type_id)) {
  case H5T_INTEGER:
    // Integer
    mem_type_id = H5T_NATIVE_INT;
    nrrd_type = get_nrrd_type<int>();
    break;

  case H5T_FLOAT:
    if (H5Tequal(type_id, H5T_IEEE_F32BE) ||
        H5Tequal(type_id, H5T_IEEE_F32LE) ||
        H5Tequal(type_id, H5T_NATIVE_FLOAT)) {
      // Float
      mem_type_id = H5T_NATIVE_FLOAT;
      nrrd_type = get_nrrd_type<float>();

    } else if (H5Tequal(type_id, H5T_IEEE_F64BE) ||
               H5Tequal(type_id, H5T_IEEE_F64LE) ||
               H5Tequal(type_id, H5T_NATIVE_DOUBLE) ||
               H5Tequal(type_id, H5T_NATIVE_LDOUBLE)) {
      // Double
      mem_type_id = H5T_NATIVE_DOUBLE;
      nrrd_type = get_nrrd_type<double>();

    } else {
      error("Undefined HDF5 float");
      return NULL;
    }
    break;

  case H5T_COMPOUND:
    error("At this time HDF5 Compound types can not be converted into Nrrds.");
    return NULL;
    break;

  case H5T_REFERENCE:
    error("At this time HDF5 Reference types are not followed.");
    error("Please select the actual object.");
    return NULL;
    break;

  default:
    error("Unknown or unsupported HDF5 data type");
    return NULL;
  }

  hsize_t size = H5Tget_size(type_id);

  if( H5Tget_size(type_id) > H5Tget_size(mem_type_id) )
    size = H5Tget_size(type_id);
  else
    size = H5Tget_size(mem_type_id);

  if( size == 0 ) {
    error( "Null data size. " );
    return NULL;
  }

  H5Tclose(type_id);

  /* Get the rank (number of dims) in the space. */
  int ndims = H5Sget_simple_extent_ndims(file_space_id);

  hsize_t *count = new hsize_t[ndims > 0 ? ndims : 1];
  hsize_t *dims = new hsize_t[ndims > 0 ? ndims : 1];

  if (H5Sis_simple(file_space_id)) {
    if (ndims == 0) {
      /* scalar dataspace */
      ndims = 1;

      dims[0] = 1;
      count[0] = 1;

      if( (data = new char[size]) == NULL ) {
        error( "Can not allocate enough memory for the data" );
        return NULL;
      }

      if( (status = H5Dread(ds_id, mem_type_id,
                            H5S_ALL, H5S_ALL, H5P_DEFAULT,
                            data)) < 0 ) {
        error( "Error reading dataset." );
        delete[] data;
        return NULL;
      }
    } else {
      /* simple dataspace */

      /* Get the dims in the space. */
      int ndim = H5Sget_simple_extent_dims(file_space_id, dims, NULL);

      if( ndim != ndims ) {
        error( "Data dimensions not match. " );
        return NULL;
      }

      for( int ic=0; ic<gui_ndims_.get(); ic++ ) {
        if( (unsigned int) gui_dims_[ic]->get() != dims[ic] ) {
          error( "Data do not have the same number of elements. " );
          return NULL;
        }
      }

      hsize_t *start  = new hsize_t[ndims];
      hsize_t *stride = new hsize_t[ndims];
      hsize_t *block  = new hsize_t[ndims];

      for( int ic=0; ic<gui_ndims_.get(); ic++ ) {
        start[ic]  = gui_starts_[ic]->get();
        stride[ic] = gui_strides_[ic]->get();
        count[ic]  = gui_counts_[ic]->get();
        block[ic]  = 1;
      }

      for( int ic=gui_ndims_.get(); ic<ndims; ic++ ) {
        start[ic]  = 0;
        stride[ic] = 1;
        count[ic]  = dims[ic];
        block[ic]  = 1;
      }

      if( (status = H5Sselect_hyperslab(file_space_id, H5S_SELECT_SET,
          start, stride, count, block)) < 0 ) {
        error( "Can not select data slab requested." );
        delete[] start;
        delete[] stride;
        delete[] block;
        return NULL;
      }

      hid_t mem_space_id = H5Screate_simple (ndims, count, NULL );

      for( int d=0; d<ndims; d++ ) {
        start[d] = 0;
        stride[d] = 1;
      }

      if( (status = H5Sselect_hyperslab(mem_space_id, H5S_SELECT_SET,
          start, stride, count, block)) < 0 ) {
        error( "Can not select memory for the data slab requested." );
        delete[] start;
        delete[] stride;
        delete[] block;
        return NULL;
      }

      for( int ic=0; ic<ndims; ic++ )
        size *= count[ic];

      if( (data = new char[size]) == NULL ) {
        error( "Can not allocate enough memory for the data" );
        delete[] start;
        delete[] stride;
        delete[] block;
        return NULL;
      }

      if( (status = H5Dread(ds_id, mem_type_id,
          mem_space_id, file_space_id, H5P_DEFAULT,
          data)) < 0 ) {
        error( "Can not read the data slab requested." );
        delete[] start;
        delete[] stride;
        delete[] block;
        delete[] data;
        return NULL;
      }

      /* Terminate access to the data space. */
      if( (status = H5Sclose(mem_space_id)) < 0 ) {
        error( "Can not cloase the memory data slab requested." );
        delete[] start;
        delete[] stride;
        delete[] block;
        delete[] data;
        return NULL;
      }

      delete[] start;
      delete[] stride;
      delete[] block;
    }
  }

  // Stuff the data into the NRRD.
  NrrdData *nout = new NrrdData();

  // If the user asks to assume vector or matrix data, the assumption
  // is based on the size of the last dimension of the hdf5 data and
  // will be in the first dimension of the nrrd
  int sz_last_dim = 1;
  if (gui_assume_svt_.get())
    sz_last_dim = dims[ndims-1];

  // The nrrd ordering is opposite of HDF5 ordering so swap the dimensions
  for(int i=0; i<ndims/2; i++ ) {
    int swap = count[i];
    count[i] = count[ndims-1-i];
    count[ndims-1-i] = swap;
  }

  size_t sz[NRRD_DIM_MAX];
  unsigned int centers[NRRD_DIM_MAX];

  for( int i=0; i<ndims; i++ ) {
    sz[i] = count[i];
    centers[i] = nrrdCenterNode;
  }

  nrrdWrap_nva(nout->nrrd_, data, nrrd_type, ndims, sz);
  nrrdAxisInfoSet_nva(nout->nrrd_, nrrdAxisInfoCenter, centers);

  // Remove all of the tcl special characters.
  string nrrdName = group + "-" + dataset;

  std::string::size_type pos;
  while( (pos = nrrdName.find(":")) != string::npos )
    nrrdName.replace( pos, 1, "-" );
  while( (pos = nrrdName.find("/")) != string::npos )
    nrrdName.replace( pos, 1, "-" );
  while( (pos = nrrdName.find("[")) != string::npos )
    nrrdName.replace( pos, 1, "_" );
  while( (pos = nrrdName.find("]")) != string::npos )
    nrrdName.erase( pos, 1 );
  while( (pos = nrrdName.find(" ")) != string::npos )
    nrrdName.replace( pos, 1, "_" );


  switch (sz_last_dim) {
  case 3: // Vector data
    nrrdName += ":Vector";
    nout->nrrd_->axis[0].kind = nrrdKind3Vector;
    break;

  case 6: // Matrix data
    nrrdName += ":Matrix";
    nout->nrrd_->axis[0].kind = nrrdKind3DSymMatrix;
    break;

  case 9: // Matrix data
    nrrdName += ":Matrix";
    nout->nrrd_->axis[0].kind = nrrdKind3DMatrix;
    break;

  default: // treat the rest as Scalar data
    nrrdName += ":Scalar";
    nout->nrrd_->axis[0].kind = nrrdKindDomain;
    break;
  };

  for( int i=1; i<ndims; i++ )
    nout->nrrd_->axis[i].kind = nrrdKindDomain;


  nout->set_property( "Name", nrrdName, false );

  delete[] dims;
  delete[] count;

  // Add the attributs from the dataset.
  if( H5Aiterate(ds_id, H5_INDEX_NAME, H5_ITER_NATIVE, 0,
                 add_attribute, nout) < 0 ) {
    error( HDF5Attribute_error_msg );
  }

  std::string parent = group;

  // Add the attributs from the parents.
  while( parent.length() > 0 ) {

    hid_t p_id = H5Gopen(file_id, parent.c_str(), H5P_DEFAULT);

    /* Open the group in the file. */
    if( p_id < 0 ) {
      error( "Error opening group. " );
    } else if( H5Aiterate(p_id, H5_INDEX_NAME, H5_ITER_NATIVE, 0,
                          add_attribute, nout) < 0 ) {
      error( HDF5Attribute_error_msg );
    } else {

      /* Terminate access to the group. */
      if( (status = H5Gclose(p_id)) < 0 )
        error( "Can not close file space." );
    }

    // Remove the last group name from the path.
    std::string::size_type pos = parent.find_last_of("/");
    parent.erase( pos, parent.length()-pos);
  }

  // Add the attributs from the top level.
  parent = "/";

  hid_t p_id = H5Gopen(file_id, parent.c_str(), H5P_DEFAULT);

  /* Open the group in the file. */
  if( p_id < 0 ) {
    error( "Error opening group. " );
  } else if( H5Aiterate(p_id, H5_INDEX_NAME, H5_ITER_NATIVE, 0,
      add_attribute, nout) < 0 ) {
    error( HDF5Attribute_error_msg );
  } else {

    /* Terminate access to the group. */
    if( (status = H5Gclose(p_id)) < 0 )
      error( "Can not close file space." );
  }


  /* Terminate access to the data space. */
  if( (status = H5Sclose(file_space_id)) < 0 )
    error( "Can not close file space." );

  /* Terminate access to the dataset. */
  if( (status = H5Dclose(ds_id)) < 0 )
    error( "Can not close file space." );

  /* Terminate access to the group. */
  if( (status = H5Gclose(g_id)) < 0 )
    error( "Can not close file space." );

  /* Terminate access to the file. */
  if( (status = H5Fclose(file_id)) < 0 )
    error( "Can not close file space." );

  return NrrdDataHandle(nout);
#else
  return NULL;
#endif
}

void ReadHDF5File::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2){
    args.error("ReadHDF5File needs a minor command");
    return;
  }

  if (args[1] == "check_dumpfile") {
#ifdef HAVE_HDF5
    gui_filename_.reset();

    string filename(gui_filename_.get());

    if( filename.length() == 0 )
      return;

    // Dump file name change
    string dumpname = getDumpFileName( filename );

    if( dumpname != gui_dumpname_.get() ) {
      gui_dumpname_.set( dumpname );
      gui_dumpname_.reset();
    }

    // Dump file not available or out of date .
    if( checkDumpFile( filename, dumpname ) ) {
      createDumpFile( filename, dumpname );
    }

#else
    error( "No HDF5 availible." );
#endif

  } else if (args[1] == "update_file") {
#ifdef HAVE_HDF5

    bool update = false;

    gui_filename_.reset();
    string new_filename(gui_filename_.get());

    if( new_filename.length() == 0 ) {
      error( string("No HDF5 file.") );
      return;
    }

    // Read the status of this file so we can compare modification timestamps
#ifdef HAVE_STAT64
    struct stat64 buf;
    if (stat64(new_filename.c_str(), &buf) == -1) {
#else
    struct stat buf;
    if (stat(new_filename.c_str(), &buf) == -1) {
#endif
      error( string("Update - File not found ") + new_filename );
      return;
    }

    // If we haven't read yet, or if it's a new filename,
    //  or if the datestamp has changed -- then read..
    time_t new_filemodification = buf.st_mtime;

    // Dump file name change
    string new_dumpname = getDumpFileName( new_filename );

    if( new_filename         != sel_filename_ ||
        new_filemodification != sel_filemodification_) {

      sel_filename_         = new_filename;
      sel_filemodification_ = new_filemodification;

      update = true;

    } else {

      update = checkDumpFile( new_filename, new_dumpname );
    }

    if( update ) {
      createDumpFile( new_filename, new_dumpname  );

      // Update the treeview in the GUI.
      ostringstream str;
      str << get_id() << " build_tree " << new_dumpname;

      TCLInterface::execute(str.str().c_str());

      // Update the dims in the GUI.
      gui_ndims_.set( 0 );
      gui_ndims_.reset();
      TCLInterface::execute(get_id() + " set_size ");
    }

#else
    error( "No HDF5 availible." );
#endif

  } else if (args[1] == "update_selection") {
#ifdef HAVE_HDF5
    gui_filename_.reset();
    gui_datasets_.reset();
    string new_filename(gui_filename_.get());
    string new_datasets(gui_datasets_.get());

    if( new_datasets.length() == 0 )
      return;

    vector<string> pathList;
    vector<string> datasetList;

    parseDatasets( new_datasets, pathList, datasetList );

    if( new_datasets != sel_datasets_ ) {
      sel_datasets_ = new_datasets;

      bool update = 0;

      for( unsigned int ic=0; ic<pathList.size(); ic++ ) {
        vector<int> dims =
          getDatasetDims( new_filename, pathList[ic], datasetList[ic] );

        if(ic == 0) {
          if( (unsigned int) gui_ndims_.get() != dims.size() ) {
            update = true;
            gui_ndims_.set( dims.size() );
            gui_ndims_.reset();
          }

          for( int jc=0; jc<gui_ndims_.get() && jc<MAX_DIMS; jc++ ) {

            if( gui_dims_[jc]->get() != dims[jc] ) {
              update = true;
              gui_dims_[jc]->set( dims[jc] );
              gui_dims_[jc]->reset();
            }
          }
        } else {
          if( (unsigned int) gui_ndims_.get() > dims.size() ) {
            update = true;
            gui_ndims_.set( dims.size() );
            gui_ndims_.reset();
          }

          for( int jc=0; jc<gui_ndims_.get() && jc<MAX_DIMS; jc++ ) {

            if( gui_dims_[jc]->get() != dims[jc] ) {
              update = true;
              gui_ndims_.set( jc );
              gui_ndims_.reset();
              break;
            }
          }
        }
      }

      // Update the dims in the GUI.
      if( update ) {
        TCLInterface::execute(get_id() + " set_size ");
      }
    }

    gui_time_series_.reset();

    // If time_series is set determine the number of time slices.
    if( gui_time_series_.get() == 1 ) {
      vector< vector<string> > time_slice_paths;
      vector< vector<string> > time_slice_datasets;

      int ntime_slices =
          parseTimeSeriesDatasets( pathList, datasetList,
          time_slice_paths, time_slice_datasets);

      if( gui_selectable_max_.get() != ntime_slices-1 ) {
        gui_selectable_max_.set(ntime_slices-1);
        gui_selectable_max_.reset();
      }
    }
#else
    error( "No HDF5 availible." );
#endif

  } else {
    Module::tcl_command(args, userdata);
  }
}


string
ReadHDF5File::getDumpFileName( string filename ) {

  string dumpname;

  std::string::size_type pos = filename.find_last_of( "/" );

  if( pos == string::npos )
    pos = 0;
  else
    pos++;

  char* tmpdir = getenv( "SCIRUN_TMP_DIR" );

  if( tmpdir )
    dumpname = tmpdir + string( "/" );
  else
    dumpname = string( "/tmp/" );

  dumpname.append( filename, pos, filename.length()-pos );
  dumpname.append( ".dump" );

  return dumpname;
}

bool
ReadHDF5File::checkDumpFile( string filename, string dumpname ) {

  bool recreate = false;

  // Read the status of this file so we can compare modification timestamps
#ifdef HAVE_STAT64
  struct stat64 buf;
  if (stat64(filename.c_str(), &buf) == -1)
#else
  struct stat buf;
  if (stat(filename.c_str(), &buf) == -1)
#endif
  {
    error( string("HDF5 File not found ") + filename );
    return false;
  }

  time_t filemodification = buf.st_mtime;
  time_t dumpfilemodification = 0;

  // Read the status of this dumpfile so we can compare modification timestamps
#ifdef HAVE_STAT64
  if (stat64(dumpname.c_str(), &buf) == -1)
#else
  if (stat(dumpname.c_str(), &buf) == -1)
#endif
  {
    warning( string("HDF5 Dump File not found ") + dumpname +
       " ... recreating.");
    recreate = true;
  } else {

    dumpfilemodification = buf.st_mtime;

    if( dumpfilemodification < filemodification ) {
      warning( string("HDF5 Dump File is out of date ") + dumpname +
         " ... recreating.");
      recreate = true;
    }
  }

  return recreate;
}


int
ReadHDF5File::createDumpFile( string filename, string dumpname ) {

#ifdef HAVE_HDF5
  std::ofstream sPtr( dumpname.c_str() );

  if( !sPtr ) {
    error( string("Unable to open output file: ") + dumpname );
    TCLInterface::execute( "reset_cursor" );
    return -1;
  }

  WriteHDF5DumpFile hdf( &sPtr );

  if( hdf.file( filename ) < 0 ) {
    error( hdf.error() );
    TCLInterface::execute( "reset_cursor" );

    sPtr.flush();
    sPtr.close();

    return -1;
  }

  sPtr.flush();
  sPtr.close();

#else
  error("No HDF5 available.");
#endif

  return 0;
}


int
ReadHDF5File::increment(int which, int lower, int upper)
{
  // Do nothing if no range.
  if (upper == lower) {
    if (gui_playmode_.get() == "once")
      gui_execmode_.set( "stop" );
    return upper;
  }

  const int inc_amount = Max(1, Min(upper, gui_inc_amount_.get()));

  which += inc_ * inc_amount;

  if (which > upper) {
    if (gui_playmode_.get() == "bounce1") {
      inc_ *= -1;
      return increment(upper, lower, upper);
    } else if (gui_playmode_.get() == "bounce2") {
      inc_ *= -1;
      return upper;
    } else {
      if (gui_playmode_.get() == "once")
        gui_execmode_.set( "stop" );
      return lower;
    }
  }

  if (which < lower) {
    if (gui_playmode_.get() == "bounce1") {
      inc_ *= -1;
      return increment(lower, lower, upper);
    } else if (gui_playmode_.get() == "bounce2") {
      inc_ *= -1;
      return lower;
    } else {
      if (gui_playmode_.get() == "once")
        gui_execmode_.set( "stop" );
      return upper;
    }
  }

  return which;
}

void
ReadHDF5File::getTtimeSeriesSlice( string new_filename,
           vector< vector<string> >& time_slice_paths,
           vector< vector<string> >& time_slice_datasets )
{
  // Get the current start and end.
  const int start = gui_range_min_.get();
  const int end   = gui_range_max_.get();

  int lower = start;
  int upper = end;
  if (lower > upper) {int tmp = lower; lower = upper; upper = tmp; }

  // Update the increment.
  if (gui_playmode_.get() == "once" || gui_playmode_.get() == "loop")
    inc_ = (start>end)?-1:1;

  // If the current value is invalid, reset it to the start.
  if (gui_current_.get() < lower || upper < gui_current_.get()) {
    gui_current_.set(start);
    inc_ = (start>end)?-1:1;
  }

  // Cache execmode and reset it in case we bail out early.
  const string execmode = gui_execmode_.get();

  int which = gui_current_.get();

  // If updating, we're done for now.
  if (execmode == "update") {

  } else if (execmode == "step") {
    inc_ = 1;
    which = increment(gui_current_.get(), lower, upper);

    ReadandSendData( new_filename, time_slice_paths[which],
                     time_slice_datasets[which], which );

  } else if (execmode == "stepb") {
    inc_ = -1;
    which = increment(gui_current_.get(), lower, upper);
    inc_ =  1;

    ReadandSendData( new_filename, time_slice_paths[which],
                     time_slice_datasets[which], which );

  } else if (execmode == "play") {

    if( !loop_ ) {
      if (gui_playmode_.get() == "once" && which >= end)
        which = start;

      if( gui_current_.get() != which) {
        gui_current_.set(which);
        gui_current_.reset();
      }
    }

    ReadandSendData( new_filename, time_slice_paths[which],
                     time_slice_datasets[which], which );

    // User may have changed the execmode to stop so recheck.
    gui_execmode_.reset();
    if ( true == (loop_ = (gui_execmode_.get() == "play")) ) {
      const int delay = gui_delay_.get();

      if( delay > 0) {
        Time::waitFor(delay/1000.0);
        // use this for cross platform instead of below
        //const unsigned int secs = delay / 1000;
        //const unsigned int msecs = delay % 1000;
        //if (secs)  { sleep(secs); }
        //if (msecs) { usleep(msecs * 1000); }
      }

      int next = increment(which, lower, upper);

      // Incrementing may cause a stop in the execmode so recheck.
      gui_execmode_.reset();
      if( true == (loop_ = (gui_execmode_.get() == "play")) ) {
        which = next;
        want_to_execute();
      }
    }

  } else if( execmode == "rewind" ) {

    if( gui_current_.get() != start) {
      which = start;

      ReadandSendData( new_filename, time_slice_paths[which],
                       time_slice_datasets[which], which );
    }

  } else if( execmode == "fforward" ) {

    if( gui_current_.get() != end) {
      which = end;

      ReadandSendData( new_filename, time_slice_paths[which],
                       time_slice_datasets[which], which );
    }

  } else if( inputs_changed_ ) {

    ReadandSendData( new_filename, time_slice_paths[which],
                     time_slice_datasets[which], which );

    if (gui_playmode_.get() == "inc_w_exec") {
      inc_ =  1;
      which = increment(which, lower, upper);
    }
  }

  if( gui_current_.get() != which) {
    gui_current_.set(which);
    gui_current_.reset();
  }
}

} // End namespace SCIRun
