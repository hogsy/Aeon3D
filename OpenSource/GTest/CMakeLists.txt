add_executable(GTest WIN32
        DrvMenu/DrvList.c
        FX/corona.c
        FX/DynLight.c
        FX/electric.c
        FX/Fx.c
        FX/ModelCtl.c
        Game/_bot.c
        Game/Attacker.c
        Game/bot.c
        Game/Genvsi.c
        Game/GMain.c
        Game/Items.c
        Game/Level.c
        Game/PathPt.c
        Game/Track.c
        Game/Weapons.c
        Procedurals/bumpmap.c
        Procedurals/ElectricFx.c
        Procedurals/fire.c
        Procedurals/gebmutil.c
        Procedurals/particles.c
        Procedurals/plasma.c
        Procedurals/proceng.c
        Procedurals/proceng_tables.c
        Procedurals/procutil.c
        Procedurals/setformat.c
        Procedurals/smoke.c
        Procedurals/Water.c
        AutoSelect.c
        Buffer.c
        cd.c
        Client.c
        Console.c
        GameMgr.c
        Genvs.c
        GMenu.c
        Host.c
        ipaddr.c
        Menu.c
        modelist.c
        NetMgr.c
        Server.c
        Text.c
        VidMode.c
)

target_sources(GTest PRIVATE DrvMenu/genvs.rc)
target_link_libraries(GTest Core Winmm Ws2_32)

add_dependencies(GTest GLDrv D3DDrv)
