#include "Halide.h"
#include <math.h>

namespace {

class LucasKanade : public Halide::Generator<LucasKanade> {
public:
    Input<Buffer<uint8_t>> input0{"input0", 2};
    Input<Buffer<uint8_t>> input1{"input1", 2};
    Output<Buffer<float>> output{"output", 3};

//    Func qr_decomposition(Func A, int width, int height) {
//        Func Q("Q");
//        Func R("R");
//        Func QR("QR");
//
//        Var m("m"), n("n"), x("x"), y("y"), k("k");
//
//        RDom cols = RDom(0, height); // collapses column
//        RDom rows = RDom(0, width); // collapses row
//
//        // (Q(i,1) = A(i,1) / norm(A(:,1))
//        Q(m,n,x,y) = 0.0f;
//        Q(m,n,0,y) = A(m,n,0,y) / sqrt(sum(pow(A(m,n,0,cols.x),2)));
//
//        for (int i = 1; i < width; i++) {
//            Func summ("summ");
//            Func z("z");
//            Func Q2("Q2");
//
//            summ(m,n,x) = 0.0f;
//            summ(m,n,x) += sum(A(m,n,i,cols.x)*Q(m,n,x,cols.x))*Q(m,n,x,cols.x);
//
//            z(m,n,x) = A(m,n,i,x) - summ(m,n,x);
//
//            // Q(j,i) = z(j)/norm(z)
//            Q2(m,n,x,y) = Q(m,n,x,y);
//            Q2(m,n,i,y) = z(m,n,y)/sqrt(sum(pow(z(m,n,cols.x),2)));
//            Q = Q2;
//
//            if (get_target().has_feature(Target::Clockwork)) {
//                summ.update().unroll(cols.x, 8);
//            }
//        }
//
//        R(m,n,x,y) = sum(Q(m,n,cols.x,x)*A(m,n,cols.x,y));
//        QR(m,n,x,y) = 0.0f;
//        QR(m,n,x,y) += R(m,n,rows.x,y)*Q(m,n,x,rows.x);
//
//        if (get_target().has_feature(Target::Clockwork)) {
//            QR.update().unroll(rows.x, 8);
//        }
//
//        // CHECK DIMENSIONS
//        return QR;
//    }
//
//    Func qr_algorithm(Func A, int width, int height) {
//        int iters = 3;
//        for (int i = 0; i < iters; i++) {
//            A = qr_decomposition(A, width, height);
//        }
//
//        return A;
//    }

    Func min_eig2x2(Func A) {
        Func min_eig("min_eig");
        Var m("m"), n("n"), x("x"), y("y");
    
        Func b("b"), c("c");
        Func discrim("discrim");
        Func real0("real0"), real1("real1"), imag("imag");

        // lam^2 - (a+d)lam + (ad-bc) = 0
        b(m,n) = -1.0f * (A(m,n,0,0)-A(m,n,1,1));
        c(m,n) = A(m,n,0,0)*A(m,n,1,1) - A(m,n,1,0)*A(m,n,0,1);

        discrim(m,n) = b(m,n)*b(m,n) - 4.0f*c(m,n);
        imag(m,n) = select(discrim(m,n) < 0.0f, (-1.0f*discrim(m,n)) / 4.0f, 0.0f); // imag^2
        real0(m,n) = select(discrim(m,n) < 0.0f, -1.0f*b(m,n) / 2.0f, (-1.0f*b(m,n) + sqrt(discrim(m,n))) / 2.0f);
        real1(m,n) = select(discrim(m,n) < 0.0f, -1.0f*b(m,n) / 2.0f, (-1.0f*b(m,n) - sqrt(discrim(m,n))) / 2.0f);

        min_eig(m,n) = min(real0(m,n)*real0(m,n)+imag(m,n), real1(m,n)*real1(m,n)+imag(m,n));

        return min_eig;
    }

    Func inverse2x2(Func A, Func& inv) {
        Var m("m"), n("n"), x("x"), y("y");

        inv(m,n,x,y) = 0.0f;
        inv(m,n,0,0) = A(m,n,1,1);         inv(m,n,1,0) = -1.0f * A(m,n,1,0);
        inv(m,n,0,1) = -1.0f * A(m,n,0,1); inv(m,n,1,1) = A(m,n,0,0);

        // divide by determinant
        inv(m,n,x,y) /= (A(m,n,0,0)*A(m,n,1,1)-A(m,n,1,0)*A(m,n,0,1));

        return inv;
    }

    void generate() {
        // https://sandipanweb.wordpress.com/2018/02/25/implementing-lucas-kanade-optical-flow-algorithm-in-python/
        int width = 64;
        int height = 64;
        int window = 15;
        float tau = 0.01f;

        Func input0_copy("input0_copy"), input1_copy("input1_copy");
        Func hw_input0("hw_input0"), hw_input1("hw_input1");

        Func i0_norm("i0_norm"), i1_norm("i1_norm");
        Func kernel_x("kernel_x"), kernel_y("kernel_y"), kernel_t("kernel_t");
        Func fx("fx"), fy("fx"), ft("ft");
        Func u("u"), v("v");

        Func output0_copy("output0_copy"), output1_copy("output1_copy");
        Func hw_output("hw_output");
        Var w("w"), x("x"), y("y"), z("z");

        Var xi, xo, yi, yo;

        RDom conv = RDom(0, 2, 0, 2);
        RDom win = RDom(-window, window, -window, window);
        RDom matmul = RDom(0,window*2+1);

        // The algorithm
        
        input0_copy(x,y) = input0(x,y);
        input1_copy(x,y) = input1(x,y);

        hw_input0(x,y) = cast<uint16_t>(input0_copy(x,y));
        hw_input1(x,y) = cast<uint16_t>(input1_copy(x,y));

        i0_norm(x,y) = cast<float>(hw_input0(x,y))/255.0f;
        i1_norm(x,y) = cast<float>(hw_input1(x,y))/255.0f;
        
        kernel_x(x,y) = 0.0f;
        kernel_x(0,0) = -0.25f; kernel_x(1,0) = 0.25f;
        kernel_x(0,1) = -0.25f; kernel_x(1,1) = 0.25f;
        
        kernel_y(x,y) = 0.0f;
        kernel_y(0,0) = -0.25f; kernel_y(1,0) = -0.25f;
        kernel_y(0,1) = 0.25f;  kernel_y(1,1) = 0.25f;

        kernel_t(x,y) = 0.0f;
        kernel_t(0,0) = 0.25f; kernel_t(1,0) = 0.25f;
        kernel_t(0,1) = 0.25f; kernel_t(1,1) = 0.25f;

        fx(x,y) = 0.0f;
        fy(x,y) = 0.0f;
        ft(x,y) = 0.0f;

        fx(x,y) += i0_norm(x+conv.x, y+conv.y) * kernel_x(conv.x, conv.y);
        fy(x,y) += i0_norm(x+conv.y, y+conv.y) * kernel_y(conv.x, conv.y);
        ft(x,y) += i1_norm(x+conv.y, y+conv.y) * kernel_t(conv.x, conv.y)
                 - i0_norm(x+conv.y, y+conv.y) * kernel_t(conv.x, conv.y);

        //    Ix = fx[i-w:i+w+1, j-w:j+w+1].flatten()
        //    Iy = fy[i-w:i+w+1, j-w:j+w+1].flatten()
        //    It = ft[i-w:i+w+1, j-w:j+w+1].flatten()
        //
        //    b = np.reshape(It, (It.shape[0],1)) # get b here
        //    A = np.vstack((Ix, Iy)).T # get A here
        //
        //    # if threshold τ is larger than the smallest eigenvalue of A'A:
        //    if np.min(abs(np.linalg.eigvals(np.matmul(A.T, A)))) >= tau:
        //        nu = np.matmul(np.linalg.pinv(A), b) # get velocity here
        //        u[i,j]=nu[0]
        //        v[i,j]=nu[1]

        Func Ix("Ix"), Iy("Iy"), It("It");
       
        Func b("b");
        Func At("At"), AtA("AtA"), AtAinv("Ainv"), Apinv("Apinv");
       
        Func eig("eig");
        Func nu("nu");

        RDom r = RDom(0,2);
        RDom rwin = RDom(0,(2*window+1)*(2*window+1));

        Ix(x,y,z) = fx(x+(z%(2*window+1)), y+(z/(2*window+1)));
        Iy(x,y,z) = fy(x+(z%(2*window+1)), y+(z/(2*window+1)));
        It(x,y,z) = ft(x+(z%(2*window+1)), y+(z/(2*window+1)));

        b(x,y,z) = It(x,y,z);
        At(x,y,z,w) = select(w == 0, Ix(x,y,z), Iy(x,y,z));
        AtA(x,y,z,w) = 0.0f;
        AtA(x,y,z,w) += At(x,y,rwin.x,w)*At(x,y,rwin.x,z);
        Func inv("inv");
        AtAinv = inverse2x2(AtA, inv);

        Apinv(x,y,z,w) = 0.0f;
        Apinv(x,y,z,w) += AtAinv(x,y,r.x,w)*At(x,y,z,r.x);

        eig = min_eig2x2(AtA);
        nu(x,y,w) = 0.0f;
        nu(x,y,w) = Apinv(x,y,rwin.x,w)*b(x,y,rwin.x);

        u(x,y) = select(eig(x,y) >= tau, nu(x,y,0), 0.0f);
        v(x,y) = select(eig(x,y) >= tau, nu(x,y,1), 0.0f);
        //u(x,y) = eig(x,y);
        //v(x,y) = eig(x,y);

        output0_copy(x,y) = u(x,y);
        output1_copy(x,y) = v(x,y);
        //hw_output(x,y,z) = select(z <= 0, output0_copy(x,y), output1_copy(x,y));
        hw_output(x,y,z) = output0_copy(x,y) + output1_copy(x,y);
        output(x,y,z) = hw_output(x,y,z);

        if (auto_schedule) {
        } else if (get_target().has_feature(Target::CoreIR)) {
        } else if (get_target().has_feature(Target::Clockwork)) {
          
            output.bound(x, 0, width-2*window);
            output.bound(y, 0, height-2*window);
            output.bound(z, 0, 2);

            hw_output.compute_root();
            hw_output
              .tile(x, y, xo, yo, xi, yi, width-2*window, height-2*window)
              .reorder(xi, yi, z, xo, yo)
              .hw_accelerate(xi, xo);

            kernel_x.compute_at(fx, x);
            kernel_y.compute_at(fy, x);
            kernel_t.compute_at(ft, x);

            inv.compute_at(hw_output, xo);
            nu.compute_at(hw_output, xo);
            Apinv.compute_at(hw_output, xo);
            AtA.compute_at(hw_output, xo);
            nu.update().unroll(rwin.x, 8);
            Apinv.update().unroll(r.x, 2);
            AtA.update().unroll(rwin.x, 8);
            //AtA.compute_root();
            //AtA.compute_at(hw_output, Var::outermost());

            fx.compute_at(hw_output, xo);
            fy.compute_at(hw_output, xo);
            ft.compute_at(hw_output, xo);
            fx.update().unroll(conv.x, 2).unroll(conv.y, 2);
            fy.update().unroll(conv.x, 2).unroll(conv.y, 2);
            ft.update().unroll(conv.x, 2).unroll(conv.y, 2);

            i0_norm.compute_at(hw_output, xo);
            i1_norm.compute_at(hw_output, xo);
            
            input0_copy.accelerator_input();
            input1_copy.accelerator_input();

            input0_copy.compute_root();
            input1_copy.compute_root();
        } else {
            output.compute_root();
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(LucasKanade, lucas_kanade)
