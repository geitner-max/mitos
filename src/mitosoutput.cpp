#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>

#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __cplusplus >= 201703L && __has_include(<filesystem>)
#    include <filesystem>
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace fs = std::experimental::filesystem;
#  elif __has_include(<boost/filesystem.hpp>)
#    include <boost/filesystem.hpp>
     namespace fs = boost::filesystem;
#  endif
#endif

//#include <LineInformation.h> // symtabAPI
#include <CodeObject.h> // parseAPI
#include <InstructionDecoder.h> // instructionAPI
#include <Module.h>
using namespace Dyninst;
using namespace SymtabAPI;
using namespace InstructionAPI;
using namespace ParseAPI;

#include "hwloc_dump.h"
#include "x86_util.h"

#include "Mitos.h"

int Mitos_create_output(mitos_output *mout)
{
    // Set top directory name
    std::stringstream ss_dname_topdir;
    ss_dname_topdir << "mitos_" << std::time(NULL);
    mout->dname_topdir = strdup(ss_dname_topdir.str().c_str());

    // Set data directory name
    std::stringstream ss_dname_datadir;
    ss_dname_datadir << ss_dname_topdir.str() << "/data";
    mout->dname_datadir = strdup(ss_dname_datadir.str().c_str());

    // Create the directories
    int err;
    err = mkdir(mout->dname_topdir,0777);
    err |= mkdir(mout->dname_datadir,0777);


    if(err)
    {
        std::cerr << "Mitos: Failed to create output directories!\n";
        return 1;
    }

    // Create file for raw sample output
    mout->fname_raw = strdup(std::string(std::string(mout->dname_datadir) + "/raw_samples.csv").c_str());
    mout->fout_raw = fopen(mout->fname_raw,"w");
    if(!mout->fout_raw)
    {
        std::cerr << "Mitos: Failed to create raw output file!\n";
        return 1;
    }

    mout->ok = true;

    return 0;
}


int Mitos_write_sample(perf_event_sample *sample, mitos_output *mout)
{
    if(!mout->ok)
        return 1;

    Mitos_resolve_symbol(sample);

    fprintf(mout->fout_raw,
            "%lu,%s,%lu,%lu,%lu,%lu,%lu,%u,%u,%lu,%lu,%u,%lu,%lu,%d\n",
            sample->ip,
            sample->data_symbol,
            sample->data_size,
            sample->num_dims,
            sample->access_index[0],
            sample->access_index[1],
            sample->access_index[2],
            sample->pid,
            sample->tid,
            sample->time,
            sample->addr,
            sample->cpu,
            sample->weight,
            sample->data_src & 0xF,
            sample->numa_node);

    return 0;
}
