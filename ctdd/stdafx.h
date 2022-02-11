#pragma once

#include <Python.h>
#include <cmath>
#include <iostream>
#include <complex>
#include <boost/unordered_map.hpp>
#include <string>
#include <boost/container_hash/hash_fwd.hpp>
#include <vector>
#include <torch/script.h>
#include <torch/torch.h>
#include <torch/python.h>
#include "config.h"
#include "CUDAcpl.h"
#include "simpletools.h"