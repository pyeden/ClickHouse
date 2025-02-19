#include "config_functions.h"

#if USE_H3

#include <Columns/ColumnsNumber.h>
#include <DataTypes/DataTypesNumber.h>
#include <Functions/FunctionFactory.h>
#include <Functions/IFunction.h>
#include <Common/typeid_cast.h>
#include <base/range.h>

#include <h3api.h>


namespace DB
{
namespace ErrorCodes
{
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
}

namespace
{

class FunctionH3IndexesAreNeighbors : public IFunction
{
public:
    static constexpr auto name = "h3IndexesAreNeighbors";

    static FunctionPtr create(ContextPtr) { return std::make_shared<FunctionH3IndexesAreNeighbors>(); }

    std::string getName() const override { return name; }

    size_t getNumberOfArguments() const override { return 2; }
    bool useDefaultImplementationForConstants() const override { return true; }
    bool isSuitableForShortCircuitArgumentsExecution(const DataTypesWithConstInfo & /*arguments*/) const override { return false; }

    DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
    {
        const auto * arg = arguments[0].get();
        if (!WhichDataType(arg).isUInt64())
            throw Exception(
                ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT,
                "Illegal type {} of argument {} of function {}. Must be UInt64",
                arg->getName(), 1, getName());

        arg = arguments[1].get();
        if (!WhichDataType(arg).isUInt64())
            throw Exception(
                ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT,
                "Illegal type {} of argument {} of function {}. Must be UInt64",
                arg->getName(), 2, getName());

        return std::make_shared<DataTypeUInt8>();
    }

    ColumnPtr executeImpl(const ColumnsWithTypeAndName & arguments, const DataTypePtr &, size_t input_rows_count) const override
    {
        const auto * col_hindex_origin = arguments[0].column.get();
        const auto * col_hindex_dest = arguments[1].column.get();

        auto dst = ColumnVector<UInt8>::create();
        auto & dst_data = dst->getData();
        dst_data.resize(input_rows_count);

        for (size_t row = 0; row < input_rows_count; ++row)
        {
            const UInt64 hindex_origin = col_hindex_origin->getUInt(row);
            const UInt64 hindex_dest = col_hindex_dest->getUInt(row);

            UInt8 res = areNeighborCells(hindex_origin, hindex_dest);

            dst_data[row] = res;
        }

        return dst;
    }
};

}

void registerFunctionH3IndexesAreNeighbors(FunctionFactory & factory)
{
    factory.registerFunction<FunctionH3IndexesAreNeighbors>();
}

}

#endif
