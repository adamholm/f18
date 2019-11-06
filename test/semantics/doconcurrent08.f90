! Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
!
! Licensed under the Apache License, Version 2.0 (the "License");
! you may not use this file except in compliance with the License.
! You may obtain a copy of the License at
!
!     http://www.apache.org/licenses/LICENSE-2.0
!
! Unless required by applicable law or agreed to in writing, software
! distributed under the License is distributed on an "AS IS" BASIS,
! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
! See the License for the specific language governing permissions and
! limitations under the License.
!
! C1140 -- A statement that might result in the deallocation of a polymorphic 
! entity shall not appear within a DO CONCURRENT construct.
module m1
  type :: Base
    integer :: x
  end type

  type, extends(Base) :: ChildType
    integer :: y
  end type

  class(Base), allocatable :: baseVar1
  type(Base) :: baseVar2
end module m1

subroutine s1()
  ! Test deallocation of polymorphic entities caused by block exit
  use m1

  block
    ! The following should not cause problems
    integer :: outerInt

    do concurrent (i = 1:10)
      ! The following should not cause problems
      block
        integer, allocatable :: blockInt
      end block
      block
        ! Test polymorphic entities
        ! The next one's OK because it's not polymorphic
        type(Base), allocatable :: NonPolyBlockVar

        ! The next one's OK because it has the "save" attribute
        class(Base), allocatable, save :: polyBlockVar1

        class(Base), allocatable :: polyBlockVar2
!ERROR: Deallocation of a polymorphic entity caused by block exit not allowed in DO CONCURRENT
      end block
    end do
  end block

end subroutine s1

subroutine s2()
  ! Test deallocation of a polymorphic entity cause by intrinsic assignment
  use m1

  class(Base), allocatable :: localVar
  class(Base), allocatable :: localVar1
  type(Base), allocatable :: localVar2

  allocate(ChildType :: localVar)
  allocate(ChildType :: localVar1)
  allocate(Base :: localVar2)

  do concurrent (i = 1:10)
    ! Test polymorphic entities
    ! Error possible deallocation a polymorphic variable
!ERROR: Deallocation of a polymorphic entity caused by assignment not allowed in DO CONCURRENT
    localVar = localVar1

    ! Error with the destination in a module to test the message
!ERROR: Deallocation of a polymorphic entity caused by assignment not allowed in DO CONCURRENT
    baseVar1 = localVar1

    ! The next one should be OK since localVar2 is not polymorphic
    localVar2 = localVar1
  end do
end subroutine s2

subroutine s3()
  ! Test direct deallocation
  use m1

  class(Base), allocatable :: polyVar
  type(Base), allocatable :: nonPolyVar

  allocate(ChildType:: polyVar)
  allocate(nonPolyVar)

  do concurrent (i = 1:10)
    ! Deallocation of a polymorphic entity
!ERROR: Deallocation of a polymorphic entity not allowed in DO CONCURRENT
    deallocate(polyVar)
    ! Deallocation of a nonpolymorphic entity
    deallocate(nonPolyVar)
  end do
end subroutine s3
