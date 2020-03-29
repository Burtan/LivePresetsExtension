#include "gtest/gtest.h"
#include <data/models/StringProjectStateContext.h>
#include <data/models/base/ParameterInfo.h>


TEST(Persist, ParameterInfoTest) {
   auto params = ParameterInfo(nullptr);

   for (int i = 0; i < 10; i ++) {
       double val = i * i;
       params.insert(i, Parameter<double>(nullptr, i, val));
   }

   auto str = WDL_FastString();
   params.persist(str);

   auto ctx = StringProjectStateContext(str);
   auto paramsRestored = ParameterInfo(nullptr, (ProjectStateContext*) &ctx);

   ASSERT_EQ(params.size(), paramsRestored.size());
   for (auto& key : params.getKeys()) {
       ASSERT_EQ(params.at(key).mValue, paramsRestored.at(key).mValue);
   }

    for (int i = 0; i < 10; i ++) {
        double val = (i * i) / 11.0;
        params.insert(i, Parameter<double>(nullptr, i, val));
    }

    auto str2 = WDL_FastString();
    params.persist(str2);

    auto ctx2 = StringProjectStateContext(str2);
    auto paramsRestored2 = ParameterInfo(nullptr, (ProjectStateContext*) &ctx2);

    ASSERT_EQ(params.size(), paramsRestored2.size());
    for (auto& key : params.getKeys()) {
        ASSERT_NEAR(params.at(key).mValue, paramsRestored2.at(key).mValue, 0.00001);
    }
}