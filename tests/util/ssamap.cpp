#include "gtest/gtest.h"
#include "util/ssamap.hpp"


TEST(ssamap, mapInitialisation)
{
  int index;
  double d = 5;
  double n = 6;
  MiniMC::Util::SSAMapImpl<int*,double*> map;
  map.initialiseValue (&index,&d);
  EXPECT_EQ(1, map.getIndex (&index));	
  EXPECT_EQ(&d,map.lookup (&index));
}

TEST(ssamap, mapCorrectInsert)
{
  int index;
  double d = 5;
  double n = 6;
  MiniMC::Util::SSAMapImpl<int*,double*> map;
  map.initialiseValue (&index,&d);
  map.updateValue (&index,&n);
  EXPECT_EQ(2, map.getIndex (&index));	
  EXPECT_EQ(&n,map.lookup (&index));

}
