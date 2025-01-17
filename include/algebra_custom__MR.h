#pragma once

#include <concepts>
#include <initializer_list>
#include <iostream>
#include <tuple>
#include <utility>

#include "space_custom_MR.h" // points and vectors

namespace NAlgebra__MR{
    template< std::size_t R, std::size_t C > requires (R > 0U) && (C > 0U)
    //using Matrix = std::array<std::array<long double, C>, C>;
    struct Matrix{
        using Value = long double;
        using RowArray = std::array<Value,C>;

        std::array<RowArray, R> values;

        Matrix() = default;

        Matrix(std::initializer_list<RowArray> rows) {
            auto last = rows.end();
            if (std::distance(rows.begin(), last) > R){
                last = std::next( rows.begin(), R );
            }

            std::size_t i = 0;
            std::for_each(rows.begin(), last, [this, &i](const auto &row){values[i++] = row;});
        }

        Value& operator[](const NSpace__MR::PointUnsign p){return values[p.x][p.y];}
        Value operator[](const NSpace__MR::PointUnsign p) const{return values[p.x][p.y];}

        Value& get(NSpace__MR::CoordUnsign x, NSpace__MR::CoordUnsign y){return values[x][y];}

        Value get(NSpace__MR::CoordUnsign x, NSpace__MR::CoordUnsign y) const {return values[x][y];}

        Matrix<C,R> transpostBuild() const{
            return buildTranspost();
        }

        Matrix<C,R> buildTranspost() const{
            Matrix<C,R> res;

            for(std::size_t i = 0; i < R; ++i){
                for(std::size_t j = 0; j < C; ++j){
                    res.values[j][i] = values[i][j];
                }
            }

            return res;
        }

        template<std::size_t C2> requires (C2 > 0)
        Matrix<R,C2> operator*(const Matrix<C,C2>& m2) const{
            auto mt = m2.buildTranspost();

            Matrix<R,C2> res;

            for(std::size_t i = 0; i < R; ++i){
                for(std::size_t j = 0; j < C2; ++j){
                    res[i][j] = NSpace__MR::scalarProd(values[i], mt.values[j]);
                }
            }

            return res;
        }

        auto& operator[](const std::size_t i) {return values[i];} // row
        const auto& operator[](const std::size_t i) const{return values[i];} // row
    };


    template<std::size_t N> requires (N > 0U)
    struct SquareMatrix : public Matrix<N,N>{
        using MatrixN = Matrix<N,N>;
        using RowArray = typename MatrixN::RowArray;
        using Value = typename MatrixN::Value;

        using MatrixN::MatrixN;
        SquareMatrix() = default;

        template<typename BaseMatrix>
        requires std::same_as<std::decay_t<BaseMatrix>, MatrixN>
        SquareMatrix(BaseMatrix&& matrix) : MatrixN(std::forward<MatrixN>(matrix)) {};

        SquareMatrix(std::initializer_list<RowArray> rows) {
            auto last = rows.end();
            if (std::distance(rows.begin(), last) > N){
                last = std::next( rows.begin(), N );
            }

            std::size_t i = 0;
            std::for_each(rows.begin(), last, [this, &i](const auto &row){(*this)[i++] = row;});
        }

        constexpr static SquareMatrix buildIdentity(){
            SquareMatrix id;
            for(std::size_t i = 0; i < N; ++i){
                for(std::size_t j = 0; j < N; ++j){
                    id[i][j] = !!(i == j); // 0 or 1
                }
            }
            return id;
        }

        constexpr static SquareMatrix Identity = buildIdentity();

        void transpose(){
            // TODO: understand why without the qualifcation the compiler gets:
            // ' there are no arguments to ‘buildTranspost’ that depend on a template parameter,
            //   so a declaration of ‘buildTranspost’ must be available [-fpermissive] '
            (*this) = MatrixN::buildTranspost();
        }

        //SquareMatrix operator*(const SquareMatrix& m2) const
        //{ return this->MatrixN::operator*(m2); }

        auto inverseBuild() const
        {return buildInverse();}

        // 0 <-> not invertible
        // last: determinant
        std::pair<SquareMatrix, Value> buildInverse() const{
            Value det{1};

            SquareMatrix res = buildIdentity();

            // use Gauss algorithm
            SquareMatrix copy = *this;

            // TODO: add anti-overflow checks.

            // clear the bottom-left part
            auto itFirstNextRow = copy.values.begin();
            for(std::size_t i = 0; i < N; ++i){
                // find any row with i-th element non-zero,
                // among the rows with index >= i.
                
                auto itRow = std::find_if( itFirstNextRow, copy.values.end(),
                                           [i](const auto& row){
                                               return std::abs(row[i]) > std::numeric_limits<Value>::epsilon();
                                           });

                if (itRow == copy.values.end()){
                    // not invertible
                    return {SquareMatrix{}, 0};
                }

                if (itRow != itFirstNextRow){
                    // swap rows. TODO: check this is correct.
                    std::swap(*itRow, *itFirstNextRow);

                    auto j = i + std::distance(itFirstNextRow, itRow);

                    std::swap(res[i], res[j]); // replicate on result matrix

                    det *= -1;
                }

                // now 'clear' the i-th element of all next rows.
                for(std::size_t k = i+1; k < N; ++k){
                    auto f = copy[k][i] / copy[i][i];

                    for(std::size_t j = i; j < N; ++j){
                        copy[k][j] -= (copy[i][j] * f); // 0 in [k][i]
                    }
                     // replicate on result matrix (full row)
                    for(std::size_t j = 0; j < N; ++j){
                        res[k][j] -= (res[i][j] * f);
                    }
                }

                // now fix [i,i] as 1 by scaling the i-th row itself.

                // replicate on result matrix (full row) (before [i][i] changes)
                det *= copy[i][i];

                for(std::size_t j = 0; j < N; ++j){
                    res[i][j] /= copy[i][i];
                }
                // j in reverse order as copy[i][i] has to change as last.
                for(std::size_t j = N; j > i;){
                    --j;
                    copy[i][j] /= copy[i][i]; // 1 in [i][i]
                }

                ++itFirstNextRow;
            }

            // Now the matrix si diagonal.

            // from last to first row, clear the upper-right part of the matrix.
            for(std::size_t i = N; i > 0; ){
                --i;

                // clear the i-th element of all rows before the i-th one
                for(std::size_t k = i; k > 0;){
                    --k;

                    // the operation on copy would be trivial, as only
                    // one element changes (since the i-th row is by then
                    // the same as of identity). Therefore, this operation
                    // is not performed and copy remains upper-diagonal.

                    for(std::size_t j = 0; j < N; ++j){
                        res[k][j] -= (res[i][j] * copy[k][i]);
                    }
                }
            }

            return {std::move(res), det};
        }
    };

    template< std::size_t R, std::size_t C >
    requires (R > 0U) && (C > 0U)
    std::string matrixToStr(const Matrix<R,C>& m){
        using std::literals::string_literals::operator""s;
        return std::accumulate( std::next(m.values.begin()), m.values.end(), "("s + NSpace__MR::arrayToStr(m.values[0]),
                                [](std::string acc, const auto& row){return acc + ", " + NSpace__MR::arrayToStr(row);} ) + ")";
    }

} // namespace NAlgebra__MR

template< std::size_t R, std::size_t C >
requires (R > 0U) && (C > 0U)
auto& operator<<(std::ostream &os, const NAlgebra__MR::Matrix<R,C>& m) noexcept{
    return os << NAlgebra__MR::matrixToStr<R,C>(m);
}

template< std::size_t N >
requires (N > 0U)
auto& operator<<(std::ostream &os, const NAlgebra__MR::SquareMatrix<N>& m) noexcept{
    return operator<< <N,N>(os,static_cast<const NAlgebra__MR::Matrix<N,N>&>(m));
}