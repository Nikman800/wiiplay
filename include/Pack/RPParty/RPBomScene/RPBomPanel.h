#ifndef RP_PARTY_BOM_PANEL_H
#define RP_PARTY_BOM_PANEL_H

#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

class RPBomPoolObj;

#include <Pack/RPParty/RPBomScene/RPBomObjPool.h>

class RPBomPanelPool : public RPBomObjPool {
public:
    static RPBomPanelPool* sInstance; // 8043CAD0

    virtual ~RPBomPanelPool();

    virtual void Destroy();
    virtual void PostCalcItems();

    static void CreateInstance();
    static void DestroyInstance();
};

class RPBomPanelCtrl : public RPSceneObj {
public:
    static RPBomPanelCtrl* sInstance; // 8043CAC8

    virtual ~RPBomPanelCtrl();

    virtual void Create();
    virtual void Load();
    virtual void Reset();
    virtual void Exit();

    void Move();
    void PreCalc();
    void CalcHit();
    void PostCalc();
    void CalcItems();
    void IncrementAges();
    void UpdateModels();
    void DeactivateAll();

    static void CreateInstance();
    static void DestroyInstance();

private:
    RPBomPanelPool* mpPool;
};

#endif
