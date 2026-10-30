#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpProjectionObject.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <nw4r/g3d.h>

RPGrpProjectionObject::RPGrpProjectionObject(u8 viewNo) : mViewNo(viewNo) {
    mCamera.Identity();
    mCamera.SetCameraType(RPGrpCamera::ECameraType_Anm);
}

void RPGrpProjectionObject::Calc(nw4r::g3d::Camera cam) {
    mCamera.CalcMatrix();
    mCamera.SaveCameraMatrix();

    cam.SetCameraMtxDirectly(mCamera.GetSavedCameraMatrix());
    mScreen.CopyToG3D(cam);
}

void RPGrpProjectionObject::LoadScnCamera(RPGrpHandle handle, const char* pName,
                                          f32 frame) {
    // Wii Play: the file and the scene are separate locals
    const nw4r::g3d::ResFile file =
        RPGrpModelResManager::GetCurrent()
            ->GetData<RPGrpModelResManager::Type_ResFile>(handle);
    const nw4r::g3d::ResAnmScn scn = file.GetResAnmScn(pName);

    LoadScnCameraMatrix(scn, frame);
    mScreen.LoadScnCamera(scn, mViewNo, frame);
}
