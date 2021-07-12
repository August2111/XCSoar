#==============================================================================
### NO LIBRARY: ###
set(Compatibility_SOURCES
        Compatibility/fmode.cpp
)
#==============================================================================


set(XCSOAR_LIB_LISTS
    Profile   # profile.a
    Renderer  ##??
    Widget    # libwidget.a
    Form      # form.a
    Look      # liblook.a
    ui        # screen.a
    io        # io.a
    Task      # libtask
    Airspace
    net     
    system      
    thread  
    util    
    InfoBoxes

    Computer
    CrossSection
    Gauge
    Atmosphere
    Audio
    Blackboard
    Dialogs
    FLARM
    Formatter
    Hardware
    IGC
    Input
    Job
    Language
    Logger
    lua
    Markers
    Menu
    Monitor
    NMEA
    Operation
    Plane
    Polar
    Replay
    Repository
    TeamCode
    Topography
    UIUtil
    Units
    Waypoint
    Weather
    XML

    Device
    Dialogs
    MapWindow
    Tracking

    time
)


list(APPEND XCSOAR_SOURCE_LISTS ${XCSOAR_LIB_LISTS})
