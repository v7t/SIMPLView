/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "FindAvgOrientations.h"

#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Common/Constants.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindAvgOrientations::FindAvgOrientations()
{
  m_HexOps = HexagonalOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_HexOps.get()));

  m_CubicOps = CubicOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_CubicOps.get()));

  m_OrthoOps = OrthoRhombicOps::New();
  m_OrientationOps.push_back(dynamic_cast<OrientationMath*> (m_OrthoOps.get()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindAvgOrientations::~FindAvgOrientations()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindAvgOrientations::execute()
{
  setErrorCondition(0);

  find_avgorientations();
  notify("FindAvgOrientations Completed", 0, Observable::UpdateProgressMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindAvgOrientations::find_avgorientations()
{
  DataContainer* m = getDataContainer();
  if (NULL == m)
  {
    setErrorCondition(-1);
    std::stringstream ss;
    ss << getNameOfClass() << " DataContainer was NULL";
    setErrorMessage(ss.str());
    return;
  }

  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::GrainIds, Int32ArrayType, int32_t, (m->totalpoints), grain_indicies);
  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::Phases, Int32ArrayType, int32_t, (m->totalpoints), phases);
  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::Euler1, FloatArrayType, float, (m->totalpoints), euler1s);
  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::Euler2, FloatArrayType, float, (m->totalpoints), euler2s);
  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::Euler3, FloatArrayType, float, (m->totalpoints), euler3s);
  GET_NAMED_ARRAY_SIZE_CHK(m, Voxel, DREAM3D::VoxelData::Quats, FloatArrayType, float, (m->totalpoints*5), quats);

  size_t numgrains = m->m_Grains.size();
  int phase;
  float voxquat[5];
  Ebsd::CrystalStructure xtal;
  for (size_t i = 1; i < numgrains; i++)
  {
	  m->m_Grains[i]->avg_quat[0] = 0.0;
	  m->m_Grains[i]->avg_quat[1] = 0.0;
	  m->m_Grains[i]->avg_quat[2] = 0.0;
	  m->m_Grains[i]->avg_quat[3] = 0.0;
	  m->m_Grains[i]->avg_quat[4] = 0.0;
  }
  float qr[5];
  for(int i = 0; i < m->totalpoints; i++)
  {
    if(grain_indicies[i] > 0 && phases[i] > 0)
	{
		OrientationMath::eulertoQuat(qr, euler1s[i], euler2s[i], euler3s[i]);
		phase = phases[i];
		xtal = m->crystruct[phase];
		m_OrientationOps[xtal]->getFZQuat(qr);
		quats[i*5 + 0] = 1.0;
		quats[i*5 + 1] = qr[1];
		quats[i*5 + 2] = qr[2];
		quats[i*5 + 3] = qr[3];
		quats[i*5 + 4] = qr[4];
		voxquat[0] = quats[i*5 + 0];
		voxquat[1] = quats[i*5 + 1];
		voxquat[2] = quats[i*5 + 2];
		voxquat[3] = quats[i*5 + 3];
		voxquat[4] = quats[i*5 + 4];
		m_OrientationOps[xtal]->getNearestQuat(m->m_Grains[grain_indicies[i]]->avg_quat, voxquat);
		for (int k = 0; k < 5; k++)
		{
		  m->m_Grains[grain_indicies[i]]->avg_quat[k] = m->m_Grains[grain_indicies[i]]->avg_quat[k] + quats[i*5 + k];
		}
	}
  }
  float q[5];
  float ea1, ea2, ea3;
  for (size_t i = 1; i < numgrains; i++)
  {
      q[1] = m->m_Grains[i]->avg_quat[1]/m->m_Grains[i]->avg_quat[0];
      q[2] = m->m_Grains[i]->avg_quat[2]/m->m_Grains[i]->avg_quat[0];
      q[3] = m->m_Grains[i]->avg_quat[3]/m->m_Grains[i]->avg_quat[0];
      q[4] = m->m_Grains[i]->avg_quat[4]/m->m_Grains[i]->avg_quat[0];
	  if(m->m_Grains[i]->avg_quat[0] == 0) q[1] = 0, q[2] = 0, q[3] = 0, q[4] = 1;
	  OrientationMath::QuattoEuler(q, ea1, ea2, ea3);
	  m->m_Grains[i]->euler1 = ea1;
      m->m_Grains[i]->euler2 = ea2;
      m->m_Grains[i]->euler3 = ea3;
  }
}

