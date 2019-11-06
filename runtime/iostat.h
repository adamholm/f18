#if 0 /* this header can be included into both Fortran and C */
Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines the error/end code values that can be returned
to an IOSTAT= specifier on a Fortran I/O statement.
See Fortran 2018 12.11.5.
#endif
#ifndef FORTRAN_RUNTIME_IOSTAT_H_
#define FORTRAN_RUNTIME_IOSTAT_H_
#define FORTRAN_RUNTIME_IOSTAT_INQUIRE_INTERNAL_UNIT 1
#define FORTRAN_RUNTIME_IOSTAT_END (-1)
#define FORTRAN_RUNTIME_IOSTAT_EOR (-2)
#define FORTRAN_RUNTIME_IOSTAT_FLUSH (-3)
#endif
