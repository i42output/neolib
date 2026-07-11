#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <source_location>

#include <neolib/core/numerical.hpp>
#include <neolib/core/easing.hpp>

template class neolib::basic_vector<double, 4u>;
template class neolib::basic_matrix<double, 4u, 4u>;

template neolib::mat44 neolib::math::affine_transformation_lerp<double>(neolib::vec3_range const&, neolib::vec3_range const&, neolib::vec3_range const&, double);
template neolib::mat44f neolib::math::affine_transformation_lerp<float>(neolib::vec3f_range const&, neolib::vec3f_range const&, neolib::vec3f_range const&, float);

namespace
{
    void test_assert(bool assertion, std::source_location const& location = std::source_location::current())
    {
        if (!assertion)
            throw std::logic_error("Test failed at " + std::string{ location.file_name() } + ":" + std::to_string(location.line()));
    }

    template <std::floating_point T>
    bool approx_equal(T lhs, T rhs, T epsilon = static_cast<T>(1e-9))
    {
        return std::abs(lhs - rhs) <= epsilon;
    }

    template <std::floating_point T, std::uint32_t D, typename Type>
    bool approx_equal(neolib::basic_vector<T, D, Type> const& lhs, neolib::basic_vector<T, D, Type> const& rhs, T epsilon = static_cast<T>(1e-9))
    {
        for (std::uint32_t index = 0u; index < D; ++index)
            if (!approx_equal(lhs[index], rhs[index], epsilon))
                return false;
        return true;
    }

    template <std::floating_point T, std::uint32_t D>
    bool approx_equal(neolib::basic_matrix<T, D, D> const& lhs, neolib::basic_matrix<T, D, D> const& rhs, T epsilon = static_cast<T>(1e-9))
    {
        for (std::uint32_t column = 0u; column < D; ++column)
            for (std::uint32_t row = 0u; row < D; ++row)
                if (!approx_equal(lhs[column][row], rhs[column][row], epsilon))
                    return false;
        return true;
    }

    template <std::floating_point T>
    neolib::basic_matrix<T, 4u, 4u> affine_scaling_matrix(neolib::basic_vector<T, 3u> const& s)
    {
        T const zero = neolib::math::constants::zero<T>;
        T const one = neolib::math::constants::one<T>;
        return {
            { s.x, zero, zero, zero },
            { zero, s.y, zero, zero },
            { zero, zero, s.z, zero },
            { zero, zero, zero, one } };
    }

    template <std::floating_point T>
    neolib::basic_matrix<T, 4u, 4u> affine_translation_matrix(neolib::basic_vector<T, 3u> const& t)
    {
        auto result = neolib::basic_matrix<T, 4u, 4u>::identity();
        neolib::apply_translation(result, t);
        return result;
    }

    // Reference T * R * S composed via matrix multiplication (n.b. apply_scaling only scales the
    // diagonal so is not a valid way to build R * S for a non-identity rotation).
    template <std::floating_point T>
    neolib::basic_matrix<T, 4u, 4u> affine_trs_matrix(
        neolib::basic_vector<T, 3u> const& translation, neolib::basic_vector<T, 3u> const& scaling, neolib::basic_vector<T, 3u> const& rotation)
    {
        return affine_translation_matrix(translation) * (neolib::affine_rotation_matrix(rotation) * affine_scaling_matrix(scaling));
    }
}

void TestVectors()
{
    using namespace neolib;

    vec3 const a{ 1.0, 2.0, 3.0 };
    vec3 const b{ 4.0, -5.0, 6.0 };

    test_assert(approx_equal(a + b, vec3{ 5.0, -3.0, 9.0 }));
    test_assert(approx_equal(a - b, vec3{ -3.0, 7.0, -3.0 }));
    test_assert(approx_equal(a * 2.0, vec3{ 2.0, 4.0, 6.0 }));
    test_assert(approx_equal(a.dot(b), 12.0));
    test_assert(approx_equal(a.cross(b), vec3{ 27.0, 6.0, -13.0 }));
    test_assert(approx_equal(vec3{ 3.0, 4.0, 0.0 }.magnitude(), 5.0));
    test_assert(approx_equal(vec3{ 3.0, 4.0, 0.0 }.normalized(), vec3{ 0.6, 0.8, 0.0 }));
    test_assert(approx_equal(vec3{ 1.0, 1.0, 1.0 }.distance(vec3{ 4.0, 5.0, 1.0 }), 5.0));

    // Cross product of parallel vectors is zero; a x b is orthogonal to both
    test_assert(approx_equal(a.cross(a), vec3{ 0.0, 0.0, 0.0 }));
    test_assert(approx_equal(a.cross(b).dot(a), 0.0));
    test_assert(approx_equal(a.cross(b).dot(b), 0.0));
}

void TestMatrices()
{
    using namespace neolib;

    // basic_matrix is column-major: each initializer-list is a COLUMN, m[column][row]
    mat33 const m{ { 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 }, { 7.0, 8.0, 9.0 } };
    test_assert(m[0][1] == 2.0 && m[1][0] == 4.0 && m[2][1] == 8.0);

    // M * ê_j extracts column j (column vector convention)
    test_assert(approx_equal(m * vec3{ 1.0, 0.0, 0.0 }, vec3{ 1.0, 2.0, 3.0 }));
    test_assert(approx_equal(m * vec3{ 0.0, 1.0, 0.0 }, vec3{ 4.0, 5.0, 6.0 }));

    // Transpose
    test_assert(m.transposed()[1][0] == 2.0 && m.transposed()[0][1] == 4.0);

    // Identity
    auto const& i = mat44::identity();
    vec4 const p{ 1.0, 2.0, 3.0, 1.0 };
    test_assert(approx_equal(i * p, p));

    // Composition: Rz(a) * Rz(b) == Rz(a + b), and (A * B) * v == A * (B * v)
    auto const ra = affine_rotation_matrix(vec3{ 0.0, 0.0, 0.4 });
    auto const rb = affine_rotation_matrix(vec3{ 0.0, 0.0, 0.9 });
    test_assert(approx_equal(ra * rb, affine_rotation_matrix(vec3{ 0.0, 0.0, 1.3 })));
    test_assert(approx_equal((ra * rb) * p, ra * (rb * p)));
}

void TestRotations()
{
    using namespace neolib;

    double const halfPi = boost::math::constants::half_pi<double>();

    // Rz(90 deg): x^ -> y^, y^ -> -x^
    auto const rz = rotation_matrix(vec3{ 0.0, 0.0, halfPi });
    test_assert(approx_equal(rz * vec3{ 1.0, 0.0, 0.0 }, vec3{ 0.0, 1.0, 0.0 }));
    test_assert(approx_equal(rz * vec3{ 0.0, 1.0, 0.0 }, vec3{ -1.0, 0.0, 0.0 }));

    // Rx(90 deg): y^ -> z^
    test_assert(approx_equal(rotation_matrix(vec3{ halfPi, 0.0, 0.0 }) * vec3 { 0.0, 1.0, 0.0 }, vec3{ 0.0, 0.0, 1.0 }));

    // Euler vs axis-angle agreement about z
    test_assert(approx_equal(rotation_matrix(vec3{ 0.0, 0.0, 0.7 }), rotation_matrix(vec3{ 0.0, 0.0, 1.0 }, 0.7), 1e-5));

    // Rotation matrices are orthonormal: R * R^T == I
    auto const r = rotation_matrix(vec3{ 0.3, -0.5, 0.7 });
    test_assert(approx_equal(r * r.transposed(), mat33::identity()));

    // affine_rotation_matrix embeds the 3x3 rotation with an intact w row
    vec4 const p{ 1.0, 0.0, 0.0, 1.0 };
    test_assert(approx_equal(affine_rotation_matrix(vec3{ 0.0, 0.0, halfPi }) * p, vec4{ 0.0, 1.0, 0.0, 1.0 }));

    // apply_translation adds to column 3
    auto m = mat44::identity();
    apply_translation(m, vec3{ 10.0, 20.0, 30.0 });
    test_assert(approx_equal(m * p, vec4{ 11.0, 20.0, 30.0, 1.0 }));
}

void TestEulerToQuat()
{
    using namespace neolib;

    // Quaternions from euler_to_quat are unit
    for (auto const& e : { vec3{ 0.0, 0.0, 0.0 }, vec3{ 0.3, -0.5, 0.7 }, vec3{ -2.1, 1.4, 3.0 } })
    {
        auto const q = euler_to_quat(e);
        test_assert(approx_equal(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z, 1.0));
    }

    // Identity rotation
    auto const q = euler_to_quat<double>(vec3{ 0.0, 0.0, 0.0 });
    test_assert(approx_equal(q.w, 1.0) && approx_equal(q.x, 0.0) && approx_equal(q.y, 0.0) && approx_equal(q.z, 0.0));
}

void TestAffineTransformationLerp()
{
    using namespace neolib;

    double const pi = boost::math::constants::pi<double>();

    vec3 const t0{ 10.0, -20.0, 30.0 }, t1{ -5.0, 5.0, 15.0 };
    vec3 const s0{ 2.0, 3.0, 4.0 }, s1{ 1.0, 1.0, 1.0 };
    vec3 const r0{ 0.3, -0.5, 0.7 }, r1{ -0.2, 0.4, 1.1 };

    auto const gen = affine_transformation_lerp_generator(
        vec3_range{ t0, t1 }, vec3_range{ s0, s1 }, vec3_range{ r0, r1 });

    // Endpoints reproduce T * R * S exactly (quaternion path must agree with the
    // matrix path used by affine_rotation_matrix; would catch any row/column
    // convention mismatch immediately)
    test_assert(approx_equal(gen(0.0), affine_trs_matrix(t0, s0, r0)));
    test_assert(approx_equal(gen(1.0), affine_trs_matrix(t1, s1, r1)));

    // Points remain affine: w stays 1 at all t
    for (double t = 0.0; t <= 1.0; t += 0.125)
        test_assert(approx_equal((gen(t) * vec4 { 1.0, 2.0, 3.0, 1.0 })[3], 1.0));

    // Translation and scaling interpolate linearly
    auto const mid = gen(0.5);
    test_assert(approx_equal(mid * vec4{ 0.0, 0.0, 0.0, 1.0 }, vec4{ 2.5, -7.5, 22.5, 1.0 }));

    // Slerp about a single axis is angle interpolation: 0 -> 90 deg at t = 0.5 is 45 deg
    auto const singleAxis = affine_transformation_lerp(
        vec3_range{ vec3{ 0.0, 0.0, 0.0 } }, vec3_range{ vec3{ 1.0, 1.0, 1.0 } },
        vec3_range{ vec3{ 0.0, 0.0, 0.0 }, vec3{ 0.0, 0.0, pi / 2.0 } }, 0.5);
    test_assert(approx_equal(singleAxis, affine_rotation_matrix(vec3{ 0.0, 0.0, pi / 4.0 })));

    // Shortest arc: 0 -> 270 deg goes backwards through -45 deg at t = 0.5
    auto const shortestArc = affine_transformation_lerp(
        vec3_range{ vec3{ 0.0, 0.0, 0.0 } }, vec3_range{ vec3{ 1.0, 1.0, 1.0 } },
        vec3_range{ vec3{ 0.0, 0.0, 0.0 }, vec3{ 0.0, 0.0, 3.0 * pi / 2.0 } }, 0.5);
    test_assert(approx_equal(shortestArc, affine_rotation_matrix(vec3{ 0.0, 0.0, -pi / 4.0 })));

    // Tiny rotation takes the nlerp branch (d > 0.9995): 0 -> 1 deg at t = 0.5 is ~0.5 deg
    double const oneDegree = pi / 180.0;
    auto const nlerp = affine_transformation_lerp(
        vec3_range{ vec3{ 0.0, 0.0, 0.0 } }, vec3_range{ vec3{ 1.0, 1.0, 1.0 } },
        vec3_range{ vec3{ 0.0, 0.0, 0.0 }, vec3{ 0.0, 0.0, oneDegree } }, 0.5);
    test_assert(approx_equal(nlerp, affine_rotation_matrix(vec3{ 0.0, 0.0, oneDegree / 2.0 }), 1e-6));

    // Identical endpoints (d == 1, degenerate slerp) are stable at all t
    auto const constant = affine_transformation_lerp_generator(
        vec3_range{ t0 }, vec3_range{ s0 }, vec3_range{ r0 });
    test_assert(approx_equal(constant(0.0), constant(0.5)) && approx_equal(constant(0.5), constant(1.0)));
    test_assert(approx_equal(constant(0.5), affine_trs_matrix(t0, s0, r0)));

    // Float instantiation agrees with double to float precision
    auto const genf = affine_transformation_lerp_generator(
        vec3f_range{ t0.as<float>(), t1.as<float>() }, vec3f_range{ s0.as<float>(), s1.as<float>() }, vec3f_range{ r0.as<float>(), r1.as<float>() });
    test_assert(approx_equal(genf(0.25f), gen(0.25).as<float>(), 1e-4f));
}

void TestEasing()
{
    using namespace neolib::math;

    test_assert(approx_equal(ease(easing::Linear, 0.3), 0.3));
    test_assert(approx_equal(ease(easing::InQuad, 0.5), 0.25));
    test_assert(approx_equal(ease(easing::OutQuad, 0.5), 0.75));
    test_assert(approx_equal(ease(easing::InQuad, easing::OutQuad, 0.0), 0.0));
    test_assert(approx_equal(ease(easing::InQuad, easing::OutQuad, 1.0), 1.0));
    // Inverted easings run 1 -> 0; partitioned_ease of one is V-shaped (1 -> 0 -> 1)
    test_assert(approx_equal(ease(easing::InvertedInOutQuint, 0.0), 1.0));
    test_assert(approx_equal(ease(easing::InvertedInOutQuint, 1.0), 0.0));
    test_assert(approx_equal(partitioned_ease(easing::InvertedInOutQuint, 0.0), 1.0));
    test_assert(approx_equal(partitioned_ease(easing::InvertedInOutQuint, 0.5), 0.0));
    test_assert(approx_equal(partitioned_ease(easing::InvertedInOutQuint, 1.0), 1.0));
}

int main()
{
    try
    {
        TestVectors();
        TestMatrices();
        TestRotations();
        TestEulerToQuat();
        TestAffineTransformationLerp();
        TestEasing();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Numerical tests failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "ease1:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::ease(neolib::math::easing::InvertedInOutQuint, t) << std::endl;
    std::cout << "partitioned_ease1:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::partitioned_ease(neolib::math::easing::InvertedInOutQuint, t) << std::endl;
    std::cout << "ease2:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::ease(neolib::math::easing::InQuad, neolib::math::easing::OutQuad, t) << std::endl;
    std::cout << "partitioned_ease2:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::partitioned_ease({ { neolib::math::easing::InQuad }, { neolib::math::easing::OutQuad } }, t) << std::endl;
    std::cout << "partitioned_ease3:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::partitioned_ease({ { neolib::math::easing::InQuad }, { neolib::math::easing::Linear }, { neolib::math::easing::OutQuad } }, t) << std::endl;

    std::cout << "All Numerical tests passed" << std::endl;
    return EXIT_SUCCESS;
}