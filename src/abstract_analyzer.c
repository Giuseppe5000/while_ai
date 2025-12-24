#include "../include/abstract_analyzer.h"
#include "domain/abstract_interval_domain.h"
#include "lang/cfg.h"

/*
TODO: Wrap every point of the CFG with an abstract state.
P0 is initialized with top and other points to bottom.

Then apply the worklist algorithm and return the fixpoint.
*/
