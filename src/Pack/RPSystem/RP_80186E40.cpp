// RPSysScene::drawTimeStamp and a global EGG::Vector3f[4] (Wii Sports' RP_80186E40.cpp).
// Unit 0x801865D0-0x80186754 (dtk/game_units.tsv): split from RPSysLoadScene.cpp because the
// "%s %s %s %s" string (.rodata 0x80317168) follows that file's string pool, which one
// translation unit cannot produce. Ported from doldecomp/ogws (CC0) RPSysLoadScene.cpp.
#include <Pack/RPSystem.h>

#include <revolution/OS.h>

#include <egg/math.h>

#include <cstring>

/**
 * @brief Displays the game's build date timestamp
 *
 * @param color Text color
 */
void RPSysScene::drawTimeStamp(nw4r::ut::Color color) {
    enum {
        YEARSIZE = sizeof("yyyy") - 1,
        MONTHSIZE = sizeof("mm") - 1,
        DAYSIZE = sizeof("dd") - 1,
        TIMESIZE = sizeof("hh:mm") - 1,
        BUFSIZE = 8
    };

    RP_GET_INSTANCE(RPSysTextWriter)->Begin();
    {
        RP_GET_INSTANCE(RPSysTextWriter)->SetDrawFlag(0);
        RP_GET_INSTANCE(RPSysTextWriter)->SetScale(0.65f, 0.65f);
        RP_GET_INSTANCE(RPSysTextWriter)->SetTextColor(color);

        const char* pTimeStamp =
            RP_GET_INSTANCE(RPSysSystem)->getTimeStampString();

        char year[BUFSIZE];
        std::strncpy(   //
            year,       //
            pTimeStamp, //
            YEARSIZE);

        char month[BUFSIZE];
        std::strncpy(                            //
            month,                               //
            pTimeStamp + YEARSIZE + sizeof('/'), //
            MONTHSIZE);

        char day[BUFSIZE];
        std::strncpy(                                                      //
            day,                                                           //
            pTimeStamp + YEARSIZE + sizeof('/') + MONTHSIZE + sizeof('/'), //
            DAYSIZE);

        char time[BUFSIZE];
        std::strncpy( //
            time,     //
            pTimeStamp + YEARSIZE + sizeof('/') + MONTHSIZE + sizeof('/') +
                DAYSIZE + sizeof(" \r\n") - 1,
            TIMESIZE);

        year[YEARSIZE] = 0;
        month[MONTHSIZE] = 0;
        day[DAYSIZE] = 0;
        time[TIMESIZE] = 0;

        static const char sFormat[] = "%s %s %s %s";
        RP_GET_INSTANCE(RPSysTextWriter)
            ->PrintfZeroCenter(-0.95f, -0.90f, sFormat, //
                               month, day, year, time);
    }
    RP_GET_INSTANCE(RPSysTextWriter)->End();
}

EGG::Vector3f RP_vec4Array[4];
