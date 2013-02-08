/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "VoxelDataContainerWriter.h"


#include "MXA/Utilities/MXAFileInfo.h"


#include "H5Support/H5Utilities.h"
#include "H5Support/H5Lite.h"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VoxelDataContainerWriter::VoxelDataContainerWriter() :
  AbstractFilter(),
  m_HdfFileId(-1),
  m_WriteXdmfFile(false),
  m_XdmfPtr(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VoxelDataContainerWriter::~VoxelDataContainerWriter()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::setupFilterParameters()
{
  std::vector<FilterParameter::Pointer> parameters;
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::writeFilterParameters(AbstractFilterParametersWriter* writer)
{
  /* Place code that will write the inputs values into a file. reference the
   AbstractFilterParametersWriter class for the proper API to use. */
  /*  writer->writeValue("OutputFile", getOutputFile() ); */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  VoxelDataContainer* m = getVoxelDataContainer();

  if(NULL == m)
  {
    setErrorCondition(-383);
    addErrorMessage(getHumanLabel(), "Voxel DataContainer is missing", getErrorCondition());
  }
  if(m_HdfFileId < 0)
  {
    setErrorCondition(-150);
    addErrorMessage(getHumanLabel(), "The HDF5 file id was < 0. This means this value was not set correctly from the calling object.", getErrorCondition());
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::preflight()
{
  /* Place code here that sanity checks input arrays and input values. Look at some
  * of the other DREAM3DLib/Filters/.cpp files for sample codes */
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::execute()
{
  int err = 0;
  std::stringstream ss;
  setErrorCondition(err);
  VoxelDataContainer* m = getVoxelDataContainer();
  if(NULL == m)
  {
    setErrorCondition(-999);
    notifyErrorMessage("The Voxel DataContainer Object was NULL", -999);
    return;
  }
  setErrorCondition(0);
  dataCheck(false, 1, 1, 1);
  hid_t dcGid = -1;

  int64_t volDims[3] =
  { m->getXPoints(), m->getYPoints(), m->getZPoints() };
  float spacing[3] =
  { m->getXRes(), m->getYRes(), m->getZRes() };
  float origin[3] =
  { 0.0f, 0.0f, 0.0f };

  writeXdmfGridHeader(origin, spacing, volDims);

  // Create the HDF5 Group for the Data Container
  err = H5Utilities::createGroupsFromPath(DREAM3D::HDF5::VoxelDataContainerName.c_str(), m_HdfFileId);
  if (err < 0)
  {
    ss.str("");
    ss << "Error creating HDF Group " << DREAM3D::HDF5::VoxelDataContainerName << std::endl;
    setErrorCondition(-60);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    return;
  }
  dcGid = H5Gopen(m_HdfFileId, DREAM3D::HDF5::VoxelDataContainerName.c_str(), H5P_DEFAULT );
  if (dcGid < 0)
  {
    ss.str("");
    ss << "Error opening Group " << DREAM3D::HDF5::VoxelDataContainerName << std::endl;
    setErrorCondition(-61);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    return;
  }

  // This just writes the header information
  err = writeMetaInfo(DREAM3D::HDF5::VoxelDataContainerName, volDims, spacing, origin);
  if (err < 0)
  {
    ss.str("");
    ss <<  ":Error Writing header information to output file" << std::endl;
    setErrorCondition(-62);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return;
  }

  err = writeCellData(dcGid);
  if (err < 0)
  {
    H5Gclose(dcGid); // Close the Data Container Group
    return;
  }

  err = writeFieldData(dcGid);
  if (err < 0)
  {
    H5Gclose(dcGid); // Close the Data Container Group
    return;
  }

  err = writeEnsembleData(dcGid);
  if (err < 0)
  {
    H5Gclose(dcGid); // Close the Data Container Group
    return;
  }

  // Now finally close the group and the HDf5 File
  H5Gclose(dcGid); // Close the Data Container Group

  writeXdmfGridFooter();

  notifyStatusMessage("Complete");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::setXdmfOStream(std::ostream *xdmf)
{
  m_XdmfPtr = xdmf;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::writeXdmfGridHeader(float* origin, float* spacing, int64_t* volDims)
{
  if (false == m_WriteXdmfFile || NULL == m_XdmfPtr || NULL == getVoxelDataContainer())
  {
    return;
  }
  std::ostream& out = *m_XdmfPtr;
  out << "\n  <Grid Name=\"Voxel DataContainer\" GridType=\"Uniform\">" << std::endl;
  out << "    <Topology TopologyType=\"3DCoRectMesh\" Dimensions=\"" << volDims[2] + 1 << " " << volDims[1] + 1 << " " << volDims[0] + 1 << " \"></Topology>" << std::endl;
  out << "    <Geometry Type=\"ORIGIN_DXDYDZ\">" << std::endl;
  out << "      <!-- Origin -->" << std::endl;
  out << "      <DataItem Format=\"XML\" Dimensions=\"3\">" << origin[2] << " " << origin[1] << " " << origin[0] <<  "</DataItem>" << std::endl;
  out << "      <!-- DxDyDz (Spacing/Resolution)-->" << std::endl;
  out << "      <DataItem Format=\"XML\" Dimensions=\"3\">" << spacing[2] << " " << spacing[1] << " " << spacing[0] <<  "</DataItem>" << std::endl;
  out << "    </Geometry>" << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::writeXdmfGridFooter()
{
  if (false == m_WriteXdmfFile || NULL == m_XdmfPtr || NULL == getVoxelDataContainer())
  {
    return;
  }
  std::ostream& out = *m_XdmfPtr;
  out << "  </Grid>" << std::endl;
  out << "    <!-- *************** END OF VoxelDataContainer *************** -->" << std::endl;
  out << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VoxelDataContainerWriter::writeXdmfCellData(const std::string groupName, IDataArray::Pointer array)
{
  if (m_WriteXdmfFile == false || m_XdmfPtr == NULL)
  { return; }

  VoxelDataContainer* m = getVoxelDataContainer();
  int64_t volDims[3] = { m->getXPoints(), m->getYPoints(), m->getZPoints() };

  std::ostream& out = *m_XdmfPtr;
  std::stringstream dimStr;
  int precision = 0;
  std::string xdmfTypeName;
  array->GetXdmfTypeAndSize(xdmfTypeName, precision);
  if (0 == precision)
  {
    out << "<!-- " << array->GetName() << " has unkown type or unsupported type or precision for XDMF to understand" << " -->" << std::endl;
    return;
  }

  int numComp = array->GetNumberOfComponents();
  out << "    <Attribute Name=\"" << array->GetName() << "\" ";
  if (numComp == 1)
  {
    out << "AttributeType=\"Scalar\" ";
    dimStr << volDims[2] << " " << volDims[1] << " " << volDims[0] << " ";
  }
  else
  {
    out << "AttributeType=\"Vector\" ";
    dimStr << volDims[2] << " " << volDims[1] << " " << volDims[0] << " " << numComp << " ";
  }
  out << "Center=\"Cell\">" << std::endl;
  // Open the <DataItem> Tag
  out << "      <DataItem Format=\"HDF\" Dimensions=\"" << dimStr.str() <<  "\" ";
  out << "NumberType=\"" << xdmfTypeName << "\" " << "Precision=\"" << precision << "\" >" << std::endl;

  //       <Attribute Name="Confidence Index" AttributeType="Scalar" Center="Cell">
  //<DataItem Format="HDF" Dimensions="117 201 190" NumberType="Float" Precision="4">
  ssize_t nameSize = H5Fget_name(m_HdfFileId, NULL, 0) + 1;
  std::vector<char> nameBuffer(nameSize, 0);
  nameSize = H5Fget_name(m_HdfFileId, &(nameBuffer.front()), nameSize);

  std::string hdfFileName(&(nameBuffer.front()), nameSize);
  hdfFileName = MXAFileInfo::filename(hdfFileName);

  out << "        " << hdfFileName << ":/VoxelDataContainer/" << groupName << "/" << array->GetName() << std::endl;
  //Small_IN100.dream3d:/DataContainer/CELL_DATA/Confidence Index
  out << "      </DataItem>" << std::endl;
  //     </Attribute>

  out << "    </Attribute>" << std::endl << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VoxelDataContainerWriter::writeMetaInfo(const std::string &hdfPath, int64_t volDims[3], float spacing[3], float origin[3])
{
  herr_t err = 0;
  err = createVtkObjectGroup(hdfPath, H5_VTK_STRUCTURED_POINTS);
  if (err < 0)  {
    return err;
  }
  hid_t gid = H5Gopen(m_HdfFileId, hdfPath.c_str(), H5P_DEFAULT );

  int32_t rank =1;
  hsize_t dims[1] = {3};
  err = H5Lite::writePointerDataset(gid, H5_DIMENSIONS, rank, dims, volDims);
  if (err < 0)
  {
    std::cout << "Error Writing H5_DIMENSIONS array for " << hdfPath << std::endl;
  }
  err = H5Lite::writePointerDataset(gid, H5_ORIGIN, rank, dims, origin);
  if (err < 0)
  {
    std::cout << "Error Writing H5_ORIGIN array for " << hdfPath << std::endl;
  }
  err = H5Lite::writePointerDataset(gid, H5_SPACING, rank, dims, spacing);
  if (err < 0)
  {
    std::cout << "Error Writing H5_SPACING array for " << hdfPath << std::endl;
  }
  int64_t nPoints = volDims[0] * volDims[1] * volDims[2];
  err = H5Lite::writeScalarAttribute(m_HdfFileId, hdfPath, H5_NUMBER_OF_POINTS, nPoints);
  if (err < 0)
  {
    std::cout << "Error Writing H5_NUMBER_OF_POINTS attribute for " << hdfPath << std::endl;
  }

  err |= H5Gclose(gid);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VoxelDataContainerWriter::writeCellData(hid_t dcGid)
{
  std::stringstream ss;
  int err = 0;
  VoxelDataContainer* m = getVoxelDataContainer();

  // Write the Voxel Data
  err = H5Utilities::createGroupsFromPath(H5_CELL_DATA_GROUP_NAME, dcGid);
  if(err < 0)
  {
    ss.str("");
    ss << "Error creating HDF Group " << H5_CELL_DATA_GROUP_NAME << std::endl;
    setErrorCondition(-63);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return err;
  }
  hid_t cellGroupId = H5Gopen(dcGid, H5_CELL_DATA_GROUP_NAME, H5P_DEFAULT);
  if(err < 0)
  {
    ss.str("");
    ss << "Error writing string attribute to HDF Group " << H5_CELL_DATA_GROUP_NAME << std::endl;
    setErrorCondition(-64);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return err;
  }
  NameListType names = m->getCellArrayNameList();
  for (NameListType::iterator iter = names.begin(); iter != names.end(); ++iter)
  {
    ss.str("");
    ss << "Writing Cell Data '" << *iter << "' to HDF5 File" << std::endl;
    notifyStatusMessage(ss.str());
    IDataArray::Pointer array = m->getCellData(*iter);
    err = array->writeH5Data(cellGroupId);
    if(err < 0)
    {
      ss.str("");
      ss << "Error writing array '" << *iter << "' to the HDF5 File";
      addErrorMessage(getHumanLabel(), ss.str(), err);
      setErrorCondition(err);
      H5Gclose(cellGroupId); // Close the Cell Group
      H5Gclose(dcGid); // Close the Data Container Group
      return err;
    }
    writeXdmfCellData(H5_CELL_DATA_GROUP_NAME, array);
  }
  H5Gclose(cellGroupId); // Close the Cell Group
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VoxelDataContainerWriter::writeFieldData(hid_t dcGid)
{
  std::stringstream ss;
  int err = 0;
  VoxelDataContainer* m = getVoxelDataContainer();

  // Write the Field Data
  err = H5Utilities::createGroupsFromPath(H5_FIELD_DATA_GROUP_NAME, dcGid);
  if(err < 0)
  {
    std::cout << "Error creating HDF Group " << H5_FIELD_DATA_GROUP_NAME << std::endl;
    return err;
  }
  err = H5Lite::writeStringAttribute(dcGid, H5_FIELD_DATA_GROUP_NAME, H5_NAME, H5_FIELD_DATA_DEFAULT);
  if(err < 0)
  {
    return err;
  }

  hid_t fieldGroupId = H5Gopen(dcGid, H5_FIELD_DATA_GROUP_NAME, H5P_DEFAULT);
  if(err < 0)
  {
    ss.str("");
    ss << "Error opening field Group " << H5_FIELD_DATA_GROUP_NAME << std::endl;
    setErrorCondition(-65);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return err;
  }
  NameListType names = m->getFieldArrayNameList();
  for (NameListType::iterator iter = names.begin(); iter != names.end(); ++iter)
  {
    IDataArray::Pointer array = m->getFieldData(*iter);
    err = array->writeH5Data(fieldGroupId);
    if(err < 0)
    {
      ss.str("");
      ss << "Error writing field array '" << *iter << "' to the HDF5 File";
      addErrorMessage(getHumanLabel(), ss.str(), err);
      setErrorCondition(err);
      H5Gclose(fieldGroupId); // Close the Cell Group
      H5Gclose(dcGid); // Close the Data Container Group
      return err;
    }
  }
  H5Gclose(fieldGroupId);
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VoxelDataContainerWriter::writeEnsembleData(hid_t dcGid)
{
  std::stringstream ss;
  int err = 0;
  VoxelDataContainer* m = getVoxelDataContainer();

  // Write the Ensemble data
  err = H5Utilities::createGroupsFromPath(H5_ENSEMBLE_DATA_GROUP_NAME, dcGid);
  if(err < 0)
  {
    ss.str("");
    ss << "Error creating HDF Group " << H5_ENSEMBLE_DATA_GROUP_NAME << std::endl;
    setErrorCondition(-66);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return err;
  }
  err = H5Lite::writeStringAttribute(dcGid, H5_ENSEMBLE_DATA_GROUP_NAME, H5_NAME, H5_ENSEMBLE_DATA_DEFAULT);

  hid_t ensembleGid = H5Gopen(dcGid, H5_ENSEMBLE_DATA_GROUP_NAME, H5P_DEFAULT);
  if(err < 0)
  {
    ss.str("");
    ss << "Error opening ensemble Group " << H5_ENSEMBLE_DATA_GROUP_NAME << std::endl;
    setErrorCondition(-67);
    addErrorMessage(getHumanLabel(), ss.str(), err);
    H5Gclose(dcGid); // Close the Data Container Group
    return err;
  }
  NameListType names = m->getEnsembleArrayNameList();
  for (NameListType::iterator iter = names.begin(); iter != names.end(); ++iter)
  {
    IDataArray::Pointer array = m->getEnsembleData(*iter);
    err = array->writeH5Data(ensembleGid);
    if(err < 0)
    {
      ss.str("");
      ss << "Error writing Ensemble array '" << *iter << "' to the HDF5 File";
      addErrorMessage(getHumanLabel(), ss.str(), err);
      setErrorCondition(err);
      H5Gclose(ensembleGid); // Close the Cell Group
      H5Gclose(dcGid); // Close the Data Container Group
      return err;
    }
  }

  H5Gclose(ensembleGid);

  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VoxelDataContainerWriter::createVtkObjectGroup(const std::string &hdfGroupPath, const char* vtkDataObjectType)
{
  // std::cout << "   vtkH5DataWriter::WritePoints()" << std::endl;
  herr_t err = H5Utilities::createGroupsFromPath(hdfGroupPath, m_HdfFileId);
  if (err < 0)
  {
    std::cout << "Error creating HDF Group " << hdfGroupPath << std::endl;
  }
  err = H5Lite::writeStringAttribute(m_HdfFileId, hdfGroupPath, H5_VTK_DATA_OBJECT, vtkDataObjectType );
  if(err < 0)
  {
    std::cout << "Error writing string attribute to HDF Group " << hdfGroupPath << std::endl;
  }
  return err;
}
