#ifndef MU_PROJECTSCENE_IPLAYBACKCONTROLLER_H
#define MU_PROJECTSCENE_IPLAYBACKCONTROLLER_H

#include "modularity/imoduleinterface.h"

namespace au::projectscene {
class IPlaybackController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(au::projectscene::IPlaybackController);
public:
    ~IPlaybackController() = default;

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void rewind() = 0;
};
}

#endif // MU_PROJECTSCENE_IPLAYBACKCONTROLLER_H
