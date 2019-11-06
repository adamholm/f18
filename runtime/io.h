// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Defines API between compiled code and I/O runtime library.

#ifndef FORTRAN_RUNTIME_IO_H_
#define FORTRAN_RUNTIME_IO_H_

#include "entry-names.h"
#include "iostat.h"
#include <cinttypes>
#include <cstddef>

namespace Fortran::runtime {
class Descriptor;
class NamelistGroup;
};

namespace Fortran::runtime::io {

class IoStatementState;
using Cookie = IoStatementState *;
using ExternalUnit = int;
using AsynchronousId = int;
static constexpr ExternalUnit DefaultUnit{-1};  // READ(*), WRITE(*), PRINT

extern "C" {

#define IONAME(name) RTNAME(io_##name)

// These functions initiate data transfer statements (READ, WRITE, PRINT).
// Example: PRINT *, 666 is implemented as the series of calls:
//   Cookie cookie{BeginExternalListOutput(DefaultUnit)};
//   OutputInteger64(cookie, 666);
//   EndIOStatement(cookie);

// Internal I/O initiation
// Internal I/O can loan the runtime library an optional block of memory
// in which the library can maintain state across the calls that implement
// the internal transfer; use of these blocks can reduce the need for dynamic
// memory allocation &/or thread-local storage.  The block must be sufficiently
// aligned to hold a pointer.
constexpr std::size_t RecommendedInternalIoScratchAreaBytes(
    int maxFormatParenthesesNestingDepth) {
  return 32 + 8 * maxFormatParenthesesNestingDepth;
}

// Internal I/O to/from a default-kind character scalar can avoid a
// descriptor.
Cookie IONAME(BeginInternalListOutput)(char *internal, std::size_t bytes,
    void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalListInput)(char *internal, std::size_t bytes,
    void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalFormattedOutput)(char *internal, std::size_t bytes,
    const char *format, std::size_t formatBytes, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalFormattedInput)(char *internal, std::size_t bytes,
    const char *format, std::size_t formatBytes, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);

// Internal I/O to/from character arrays &/or non-default-kind character
// requires a descriptor, which must remain unchanged until the I/O
// statement is complete.
Cookie IONAME(BeginInternalArrayListOutput)(const Descriptor &,
    void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalArrayListInput)(const Descriptor &,
    void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalArrayFormattedOutput)(const Descriptor &,
    const char *format, std::size_t formatBytes, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalArrayFormattedInput)(const Descriptor &,
    const char *format, std::size_t formatBytes, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);

// Internal namelist I/O
Cookie IONAME(BeginInternalNamelistOutput)(const Descriptor &,
    const NamelistGroup &, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalNamelistInput)(const Descriptor &,
    const NamelistGroup &, void **scratchArea = nullptr,
    std::size_t scratchBytes = 0);

// External synchronous I/O initiation
Cookie IONAME(BeginExternalListOutput)(ExternalUnit);
Cookie IONAME(BeginExternalListInput)(ExternalUnit);
Cookie IONAME(BeginExternalFormattedOutput)(
    ExternalUnit, const char *format, std::size_t);
Cookie IONAME(BeginExternalFormattedInput)(
    ExternalUnit, const char *format, std::size_t);
Cookie IONAME(BeginUnformattedOutput)(ExternalUnit);
Cookie IONAME(BeginUnformattedInput)(ExternalUnit);
Cookie IONAME(BeginNamelistOutput)(ExternalUnit, const NamelistGroup &);
Cookie IONAME(BeginNamelistInput)(ExternalUnit, const NamelistGroup &);

// Asynchronous I/O is supported (at most) for unformatted direct access
// block transfers.
AsynchronousId IONAME(BeginAsynchronousOutput)(
    ExternalUnit, std::int64_t REC, const char *, std::size_t);
AsynchronousId IONAME(BeginAsynchronousInput)(
    ExternalUnit, std::int64_t REC, char *, std::size_t);
Cookie IONAME(WaitForAsynchronousIo)(ExternalUnit, AsynchronousId);  // WAIT

// Other I/O statements
// TODO: OPEN & INQUIRE
Cookie IONAME(BeginClose)(ExternalUnit);
Cookie IONAME(BeginFlush)(ExternalUnit);
Cookie IONAME(BeginBackspace)(ExternalUnit);
Cookie IONAME(BeginEndfile)(ExternalUnit);
Cookie IONAME(BeginRewind)(ExternalUnit);

// Control list options
void IONAME(SetAdvance)(Cookie, const char *, std::size_t);
void IONAME(SetBlank)(Cookie, const char *, std::size_t);
void IONAME(SetDecimal)(Cookie, const char *, std::size_t);
void IONAME(SetDelim)(Cookie, const char *, std::size_t);
void IONAME(SetPad)(Cookie, const char *, std::size_t);
void IONAME(SetPos)(Cookie, std::int64_t);
void IONAME(SetRec)(Cookie, std::int64_t);
void IONAME(SetRound)(Cookie, const char *, std::size_t);
void IONAME(SetSign)(Cookie, const char *, std::size_t);

// Data item transfer for modes other than namelist.
// Any data object that can be passed as an actual argument without the
// use of a temporary can be transferred by means of a descriptor;
// vector-valued subscripts and coindexing will require elementwise
// transfers &/or data copies.  Unformatted transfers to/from contiguous
// blocks of local image memory can avoid the descriptor, and there
// are specializations for the most common scalar types.
//
// These functions return false when the I/O statement has encountered an error
// or end-of-file/record condition.  Once the statement has encountered
// an error, following items will be ignored; but compiled code should
// check for errors and avoid the following items when a READ statement
// might have data dependences between items, as in
//   READ(*,ERR=666) N, (A(J),J=1,N)
// or when input/output items might raise a spurious fault, as in
//   WRITE(*,ERR=666) X, CRASHIFCALLEDANDERRORPRESENT()
bool IONAME(OutputDescriptor)(Cookie, const Descriptor &);
bool IONAME(InputDescriptor)(Cookie, const Descriptor &);
bool IONAME(OutputUnformattedBlock)(Cookie, const char *, std::size_t);
bool IONAME(InputUnformattedBlock)(Cookie, char *, std::size_t);
bool IONAME(OutputInteger64)(Cookie, std::int64_t);
bool IONAME(InputInteger64)(Cookie, std::int64_t &, int kind = 8);
bool IONAME(OutputReal32)(Cookie, float);
bool IONAME(InputReal32)(Cookie, float &);
bool IONAME(OutputReal64)(Cookie, double);
bool IONAME(InputReal64)(Cookie, double &);
bool IONAME(OutputComplex32)(Cookie, float, float);
bool IONAME(OutputComplex64)(Cookie, double, double);
bool IONAME(OutputAscii)(Cookie, const char *, std::size_t);
bool IONAME(InputAscii)(Cookie, char *, std::size_t);
bool IONAME(OutputLogical)(Cookie, bool);
bool IONAME(InputLogical)(Cookie, bool &);

std::size_t IONAME(GetSize)(Cookie);  // SIZE=

// GetIoMsg() does not modify its argument unless an error or
// end-of-record/file condition is present.
void IONAME(GetIoMsg)(Cookie, char *, std::size_t);  // IOMSG=

// The "errorHandling" argument to EndIoStatement() below conveys the
// following flags that indicate how much error handling the
// compiled code is able to perform.  If an error condition has arisen
// that will not be handled, the runtime library will terminate the image
// with a message.  Note that the runtime will never terminate the image
// when (errorHandling & HasIostat) is true or when
// (errorHanding == (HasErr|HasEof|HasEor)).
enum ErrorHandlers {
  NoErrorHandler = 0,
  HasIostat = 1,  // IOSTAT= present
  HasErr = 2,  // ERR= present
  HasEnd = 4,  // END= present
  HasEor = 8,  // EOR= present
};

// This function must be called to end an I/O statement, and its
// cookie value must dead to you afterwards (although it may be
// recycled by the library and returned to serve a later I/O call).
// The return value can be used to implement IOSTAT=, ERR=, END=, & EOR=;
// store it into the IOSTAT= variable if there is one, and test
// it to implement the various branches.
int IONAME(EndIoStatement)(Cookie, int errorHandling = NoErrorHandler,
    const char *sourceFileName = nullptr, int lineNumber = 0);

// The value of IOSTAT= is zero when no error, end-of-record,
// or end-of-file condition has arisen; errors are positive values.
// (See 12.11.5 in Fortran 2018 for the complete requirements;
// these constants must match the values of their corresponding
// named constants in the predefined module ISO_FORTRAN_ENV, so
// they're actually defined in another iostat.h header file that
// can be included both here and there.)
enum Iostat {
  // Other errors have values >1
  IostatInquireInternalUnit = FORTRAN_RUNTIME_IOSTAT_INQUIRE_INTERNAL_UNIT,
  IostatOk = 0,
  IostatEnd = FORTRAN_RUNTIME_IOSTAT_END,  // end-of-file & no error
  IostatEor = FORTRAN_RUNTIME_IOSTAT_EOR,  // end-of-record & no error or EOF
  IostatFlush =
      FORTRAN_RUNTIME_IOSTAT_FLUSH,  // attempt to FLUSH an unflushable unit
};

};  // extern "C"
}
#endif
