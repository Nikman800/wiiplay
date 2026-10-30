#ifndef RP_PARTY_BIL_SCENE_H
#define RP_PARTY_BIL_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem.h>

// Forward declarations
class RPGrpCaptureTexture;
// Declared after RPBilScene (RPBilSound.h): its vtable comes before the scene's (8035DC38)
class RPBilSceneSound;

//! @addtogroup rp_bil
//! @{

/**
 * @brief Party Pack Billiards scene
 * @details vtable 8035DC48. Size 0x58, verified against Wii Play's scene creator. Configure
 * seeds the random generator, creates the RPBilGameMgr singleton (RPBilGameMgr::sInstance, which creates
 * every Billiards object), the model scenes, three screens (main, 2D and the pocket-camera
 * sub-window), the capture textures and the eleven views (3D views 0/1 use the main and aux
 * cameras, 2/3 the pocket cameras), sets the scene lights and creates the scene sound object.
 * LoadResource registers the bad_* brres files (bad_table, bad_cue, bad_ball, bad_ball_2,
 * bad_ball_0_face_2, bad_next_ball, bad_cursor, bad_arrow), rp_Bil_sound.brsar and
 * basic_billiards.breff/breft, then loads the game. Reset/Calculate forward to the game
 * manager; Exit exits and destroys it.
 */
class RPBilScene : public RPSysScene {
public:
    RPBilScene();
    virtual ~RPBilScene(); // at 0x8

    /**
     * @brief Shows or hides the players' pointer cursors (RPBilPlayerMgr::SetCursorsVisible)
     * [M] (ledger Bil_Scene_drawPass)
     */
    virtual void pauseCallBack(bool enter); // at 0x40

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54

    RPGrpCaptureTexture* GetMainCapture() const {
        return mpMainCapture;
    }
    RPGrpCaptureTexture* GetCapture48() const {
        return mpCapture48;
    }
    RPGrpCaptureTexture* GetSubWindowCapture(s32 i) const {
        return mpSubWindowCaptures[i];
    }

private:
    /**
     * @brief Capture of the 2D screen, format 6; RPBilPlayer::UserDraw captures it while
     * aiming and draws it back [M]
     */
    RPGrpCaptureTexture* mpMainCapture;          // at 0x44
    RPGrpCaptureTexture* mpCapture48;            // at 0x48 capture of the main screen [L]
    RPGrpCaptureTexture* mpSubWindowCaptures[2]; // at 0x4C pocket-camera sub-window, format 4
    RPBilSceneSound* mpSound;                    // at 0x54
};

//! @}

#endif
