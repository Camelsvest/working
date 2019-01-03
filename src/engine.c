#include "engine.h"
#include "utils/zalloc.h"

struct _engine_t {
  
};

#ifdef __cplusplus
extern "C" {
#endif

engine_t* create_engine()
{
    engine_t *engine;

	zalloc_init();
	
    engine = (engine_t *)zmalloc(sizeof(engine_t));
    return engine;
}

void destroy_engine(engine_t *engine)
{
    if (engine != NULL)
        free(engine);

	zalloc_uninit();
}



#ifdef __cplusplus
}
#endif
