#pragma once

#include "timer.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>


bool benchmark_math(u32 iterations);
bool benchmark_arith_circuit();
bool benchmark_qesa_range_proof(u32 iterations);
bool benchmark_bulletproofs(u32 iterations);
bool benchmark_shuffle_proof(u32 iterations);
