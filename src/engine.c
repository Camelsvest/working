#include <stdlib.h>
#include "engine.h"

struct _engine_t {
  
};

#ifdef __cplusplus
extern "C" {
#endif

engine_t* create_engine()
{
    engine_t *engine;

    engine = (engine_t *)malloc(sizeof(engine_t));
    return engine;
}

void destroy_engine(engine_t *engine)
{
    if (engine != NULL)
        free(engine);
}



#ifdef __cplusplus
}
#endif
