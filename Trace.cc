#include "Trace.hh"

static TraceLevel trace_level = TRACE_LEVEL_ERROR;

void trace_setLevel (TraceLevel level)
{
  trace_level = level;
}

TraceLevel trace_getLevel(void)
{
  return trace_level;
}

