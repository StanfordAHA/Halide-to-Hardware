)
{
#pragma HLS DATAFLOW
#pragma HLS INLINE region
#pragma HLS INTERFACE s_axilite port=return bundle=config
#pragma HLS INTERFACE s_axilite port=arg_0 bundle=config
#pragma HLS ARRAY_PARTITION variable=arg_0.value complete dim=0
#pragma HLS INTERFACE s_axilite port=arg_1 bundle=config
#pragma HLS ARRAY_PARTITION variable=arg_1.value complete dim=0

 // alias the arguments
 Stencil<uint16_t, 7, 116, 116> &input_host_stencil = arg_0;
 Stencil<uint16_t, 7, 114, 114> &hw_output_global_wrapper_glb_stencil = arg_1;
[, ], [, ], [, ] // produce input_host_global_wrapper.stencil
 for (int input_host_global_wrapper_s0_y = 0; input_host_global_wrapper_s0_y < 116; input_host_global_wrapper_s0_y++)
 {
  for (int input_host_global_wrapper_s0_x = 0; input_host_global_wrapper_s0_x < 116; input_host_global_wrapper_s0_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _386 = _input_host_stencil(0, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(0, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _386;
   uint16_t _387 = _input_host_stencil(1, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(1, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _387;
   uint16_t _388 = _input_host_stencil(2, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(2, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _388;
   uint16_t _389 = _input_host_stencil(3, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(3, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _389;
   uint16_t _390 = _input_host_stencil(4, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(4, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _390;
   uint16_t _391 = _input_host_stencil(5, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(5, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _391;
   uint16_t _392 = _input_host_stencil(6, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y);
   _input_host_global_wrapper_stencil(6, _input_host_global_wrapper_s0_x, _input_host_global_wrapper_s0_y) = _392;
  } // for input_host_global_wrapper_s0_x
 } // for input_host_global_wrapper_s0_y
 // consume input_host_global_wrapper.stencil
[, ], [, ], [, ] // produce hw_output.stencil
[, ], [, ], [, ] // produce input_host_global_wrapper_global_wrapper.stencil
 for (int input_host_global_wrapper_global_wrapper_s0_y = 0; input_host_global_wrapper_global_wrapper_s0_y < 116; input_host_global_wrapper_global_wrapper_s0_y++)
 {
  for (int input_host_global_wrapper_global_wrapper_s0_x = 0; input_host_global_wrapper_global_wrapper_s0_x < 116; input_host_global_wrapper_global_wrapper_s0_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _393 = _input_host_global_wrapper_stencil(0, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(0, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _393;
   uint16_t _394 = _input_host_global_wrapper_stencil(1, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(1, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _394;
   uint16_t _395 = _input_host_global_wrapper_stencil(2, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(2, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _395;
   uint16_t _396 = _input_host_global_wrapper_stencil(3, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(3, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _396;
   uint16_t _397 = _input_host_global_wrapper_stencil(4, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(4, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _397;
   uint16_t _398 = _input_host_global_wrapper_stencil(5, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(5, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _398;
   uint16_t _399 = _input_host_global_wrapper_stencil(6, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y);
   _input_host_global_wrapper_global_wrapper_stencil(6, _input_host_global_wrapper_global_wrapper_s0_x, _input_host_global_wrapper_global_wrapper_s0_y) = _399;
  } // for input_host_global_wrapper_global_wrapper_s0_x
 } // for input_host_global_wrapper_global_wrapper_s0_y
 // consume input_host_global_wrapper_global_wrapper.stencil
[, ], [, ], [, ] // produce depthwise_conv.stencil
 for (int depthwise_conv_s0_y = 0; depthwise_conv_s0_y < 114; depthwise_conv_s0_y++)
 {
  for (int depthwise_conv_s0_x = 0; depthwise_conv_s0_x < 114; depthwise_conv_s0_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _402 = (uint16_t)(0);
   _depthwise_conv_stencil(0, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _402;
   uint16_t _405 = (uint16_t)(0);
   _depthwise_conv_stencil(1, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _405;
   uint16_t _408 = (uint16_t)(0);
   _depthwise_conv_stencil(2, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _408;
   uint16_t _411 = (uint16_t)(0);
   _depthwise_conv_stencil(3, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _411;
   uint16_t _414 = (uint16_t)(0);
   _depthwise_conv_stencil(4, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _414;
   uint16_t _417 = (uint16_t)(0);
   _depthwise_conv_stencil(5, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _417;
   uint16_t _420 = (uint16_t)(0);
   _depthwise_conv_stencil(6, _depthwise_conv_s0_x, _depthwise_conv_s0_y) = _420;
  } // for depthwise_conv_s0_x
 } // for depthwise_conv_s0_y
 for (int depthwise_conv_s1_y = 0; depthwise_conv_s1_y < 114; depthwise_conv_s1_y++)
 {
  for (int depthwise_conv_s1_x = 0; depthwise_conv_s1_x < 114; depthwise_conv_s1_x++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _463 = _input_host_global_wrapper_global_wrapper_stencil(0, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _464 = (uint16_t)(24);
   uint16_t _465 = _463 * _464;
   uint16_t _466 = _depthwise_conv_stencil(0, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _467 = _depthwise_conv_s1_x + 1;
   uint16_t _468 = _input_host_global_wrapper_global_wrapper_stencil(0, _467, _depthwise_conv_s1_y);
   uint16_t _469 = (uint16_t)(30);
   uint16_t _470 = _468 * _469;
   int32_t _471 = _depthwise_conv_s1_x + 2;
   uint16_t _472 = _input_host_global_wrapper_global_wrapper_stencil(0, _471, _depthwise_conv_s1_y);
   uint16_t _473 = _472 * _464;
   int32_t _474 = _depthwise_conv_s1_y + 1;
   uint16_t _475 = _input_host_global_wrapper_global_wrapper_stencil(0, _depthwise_conv_s1_x, _474);
   uint16_t _476 = _475 * _469;
   uint16_t _477 = _input_host_global_wrapper_global_wrapper_stencil(0, _467, _474);
   uint16_t _478 = (uint16_t)(37);
   uint16_t _479 = _477 * _478;
   uint16_t _480 = _input_host_global_wrapper_global_wrapper_stencil(0, _471, _474);
   uint16_t _481 = _480 * _469;
   int32_t _482 = _depthwise_conv_s1_y + 2;
   uint16_t _483 = _input_host_global_wrapper_global_wrapper_stencil(0, _depthwise_conv_s1_x, _482);
   uint16_t _484 = _483 * _464;
   uint16_t _485 = _input_host_global_wrapper_global_wrapper_stencil(0, _471, _482);
   uint16_t _486 = _485 * _464;
   uint16_t _487 = _input_host_global_wrapper_global_wrapper_stencil(0, _467, _482);
   uint16_t _488 = _487 * _469;
   uint16_t _489 = _486 + _488;
   uint16_t _490 = _484 + _489;
   uint16_t _491 = _481 + _490;
   uint16_t _492 = _479 + _491;
   uint16_t _493 = _476 + _492;
   uint16_t _494 = _473 + _493;
   uint16_t _495 = _470 + _494;
   uint16_t _496 = _466 + _495;
   uint16_t _497 = _465 + _496;
   _depthwise_conv_stencil(0, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _497;
   uint16_t _540 = _input_host_global_wrapper_global_wrapper_stencil(1, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _541 = (uint16_t)(24);
   uint16_t _542 = _540 * _541;
   uint16_t _543 = _depthwise_conv_stencil(1, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _544 = _depthwise_conv_s1_x + 1;
   uint16_t _545 = _input_host_global_wrapper_global_wrapper_stencil(1, _544, _depthwise_conv_s1_y);
   uint16_t _546 = (uint16_t)(30);
   uint16_t _547 = _545 * _546;
   int32_t _548 = _depthwise_conv_s1_x + 2;
   uint16_t _549 = _input_host_global_wrapper_global_wrapper_stencil(1, _548, _depthwise_conv_s1_y);
   uint16_t _550 = _549 * _541;
   int32_t _551 = _depthwise_conv_s1_y + 1;
   uint16_t _552 = _input_host_global_wrapper_global_wrapper_stencil(1, _depthwise_conv_s1_x, _551);
   uint16_t _553 = _552 * _546;
   uint16_t _554 = _input_host_global_wrapper_global_wrapper_stencil(1, _544, _551);
   uint16_t _555 = (uint16_t)(37);
   uint16_t _556 = _554 * _555;
   uint16_t _557 = _input_host_global_wrapper_global_wrapper_stencil(1, _548, _551);
   uint16_t _558 = _557 * _546;
   int32_t _559 = _depthwise_conv_s1_y + 2;
   uint16_t _560 = _input_host_global_wrapper_global_wrapper_stencil(1, _depthwise_conv_s1_x, _559);
   uint16_t _561 = _560 * _541;
   uint16_t _562 = _input_host_global_wrapper_global_wrapper_stencil(1, _548, _559);
   uint16_t _563 = _562 * _541;
   uint16_t _564 = _input_host_global_wrapper_global_wrapper_stencil(1, _544, _559);
   uint16_t _565 = _564 * _546;
   uint16_t _566 = _563 + _565;
   uint16_t _567 = _561 + _566;
   uint16_t _568 = _558 + _567;
   uint16_t _569 = _556 + _568;
   uint16_t _570 = _553 + _569;
   uint16_t _571 = _550 + _570;
   uint16_t _572 = _547 + _571;
   uint16_t _573 = _543 + _572;
   uint16_t _574 = _542 + _573;
   _depthwise_conv_stencil(1, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _574;
   uint16_t _617 = _input_host_global_wrapper_global_wrapper_stencil(2, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _618 = (uint16_t)(24);
   uint16_t _619 = _617 * _618;
   uint16_t _620 = _depthwise_conv_stencil(2, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _621 = _depthwise_conv_s1_x + 1;
   uint16_t _622 = _input_host_global_wrapper_global_wrapper_stencil(2, _621, _depthwise_conv_s1_y);
   uint16_t _623 = (uint16_t)(30);
   uint16_t _624 = _622 * _623;
   int32_t _625 = _depthwise_conv_s1_x + 2;
   uint16_t _626 = _input_host_global_wrapper_global_wrapper_stencil(2, _625, _depthwise_conv_s1_y);
   uint16_t _627 = _626 * _618;
   int32_t _628 = _depthwise_conv_s1_y + 1;
   uint16_t _629 = _input_host_global_wrapper_global_wrapper_stencil(2, _depthwise_conv_s1_x, _628);
   uint16_t _630 = _629 * _623;
   uint16_t _631 = _input_host_global_wrapper_global_wrapper_stencil(2, _621, _628);
   uint16_t _632 = (uint16_t)(37);
   uint16_t _633 = _631 * _632;
   uint16_t _634 = _input_host_global_wrapper_global_wrapper_stencil(2, _625, _628);
   uint16_t _635 = _634 * _623;
   int32_t _636 = _depthwise_conv_s1_y + 2;
   uint16_t _637 = _input_host_global_wrapper_global_wrapper_stencil(2, _depthwise_conv_s1_x, _636);
   uint16_t _638 = _637 * _618;
   uint16_t _639 = _input_host_global_wrapper_global_wrapper_stencil(2, _625, _636);
   uint16_t _640 = _639 * _618;
   uint16_t _641 = _input_host_global_wrapper_global_wrapper_stencil(2, _621, _636);
   uint16_t _642 = _641 * _623;
   uint16_t _643 = _640 + _642;
   uint16_t _644 = _638 + _643;
   uint16_t _645 = _635 + _644;
   uint16_t _646 = _633 + _645;
   uint16_t _647 = _630 + _646;
   uint16_t _648 = _627 + _647;
   uint16_t _649 = _624 + _648;
   uint16_t _650 = _620 + _649;
   uint16_t _651 = _619 + _650;
   _depthwise_conv_stencil(2, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _651;
   uint16_t _694 = _input_host_global_wrapper_global_wrapper_stencil(3, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _695 = (uint16_t)(24);
   uint16_t _696 = _694 * _695;
   uint16_t _697 = _depthwise_conv_stencil(3, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _698 = _depthwise_conv_s1_x + 1;
   uint16_t _699 = _input_host_global_wrapper_global_wrapper_stencil(3, _698, _depthwise_conv_s1_y);
   uint16_t _700 = (uint16_t)(30);
   uint16_t _701 = _699 * _700;
   int32_t _702 = _depthwise_conv_s1_x + 2;
   uint16_t _703 = _input_host_global_wrapper_global_wrapper_stencil(3, _702, _depthwise_conv_s1_y);
   uint16_t _704 = _703 * _695;
   int32_t _705 = _depthwise_conv_s1_y + 1;
   uint16_t _706 = _input_host_global_wrapper_global_wrapper_stencil(3, _depthwise_conv_s1_x, _705);
   uint16_t _707 = _706 * _700;
   uint16_t _708 = _input_host_global_wrapper_global_wrapper_stencil(3, _698, _705);
   uint16_t _709 = (uint16_t)(37);
   uint16_t _710 = _708 * _709;
   uint16_t _711 = _input_host_global_wrapper_global_wrapper_stencil(3, _702, _705);
   uint16_t _712 = _711 * _700;
   int32_t _713 = _depthwise_conv_s1_y + 2;
   uint16_t _714 = _input_host_global_wrapper_global_wrapper_stencil(3, _depthwise_conv_s1_x, _713);
   uint16_t _715 = _714 * _695;
   uint16_t _716 = _input_host_global_wrapper_global_wrapper_stencil(3, _702, _713);
   uint16_t _717 = _716 * _695;
   uint16_t _718 = _input_host_global_wrapper_global_wrapper_stencil(3, _698, _713);
   uint16_t _719 = _718 * _700;
   uint16_t _720 = _717 + _719;
   uint16_t _721 = _715 + _720;
   uint16_t _722 = _712 + _721;
   uint16_t _723 = _710 + _722;
   uint16_t _724 = _707 + _723;
   uint16_t _725 = _704 + _724;
   uint16_t _726 = _701 + _725;
   uint16_t _727 = _697 + _726;
   uint16_t _728 = _696 + _727;
   _depthwise_conv_stencil(3, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _728;
   uint16_t _771 = _input_host_global_wrapper_global_wrapper_stencil(4, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _772 = (uint16_t)(24);
   uint16_t _773 = _771 * _772;
   uint16_t _774 = _depthwise_conv_stencil(4, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _775 = _depthwise_conv_s1_x + 1;
   uint16_t _776 = _input_host_global_wrapper_global_wrapper_stencil(4, _775, _depthwise_conv_s1_y);
   uint16_t _777 = (uint16_t)(30);
   uint16_t _778 = _776 * _777;
   int32_t _779 = _depthwise_conv_s1_x + 2;
   uint16_t _780 = _input_host_global_wrapper_global_wrapper_stencil(4, _779, _depthwise_conv_s1_y);
   uint16_t _781 = _780 * _772;
   int32_t _782 = _depthwise_conv_s1_y + 1;
   uint16_t _783 = _input_host_global_wrapper_global_wrapper_stencil(4, _depthwise_conv_s1_x, _782);
   uint16_t _784 = _783 * _777;
   uint16_t _785 = _input_host_global_wrapper_global_wrapper_stencil(4, _775, _782);
   uint16_t _786 = (uint16_t)(37);
   uint16_t _787 = _785 * _786;
   uint16_t _788 = _input_host_global_wrapper_global_wrapper_stencil(4, _779, _782);
   uint16_t _789 = _788 * _777;
   int32_t _790 = _depthwise_conv_s1_y + 2;
   uint16_t _791 = _input_host_global_wrapper_global_wrapper_stencil(4, _depthwise_conv_s1_x, _790);
   uint16_t _792 = _791 * _772;
   uint16_t _793 = _input_host_global_wrapper_global_wrapper_stencil(4, _779, _790);
   uint16_t _794 = _793 * _772;
   uint16_t _795 = _input_host_global_wrapper_global_wrapper_stencil(4, _775, _790);
   uint16_t _796 = _795 * _777;
   uint16_t _797 = _794 + _796;
   uint16_t _798 = _792 + _797;
   uint16_t _799 = _789 + _798;
   uint16_t _800 = _787 + _799;
   uint16_t _801 = _784 + _800;
   uint16_t _802 = _781 + _801;
   uint16_t _803 = _778 + _802;
   uint16_t _804 = _774 + _803;
   uint16_t _805 = _773 + _804;
   _depthwise_conv_stencil(4, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _805;
   uint16_t _848 = _input_host_global_wrapper_global_wrapper_stencil(5, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _849 = (uint16_t)(24);
   uint16_t _850 = _848 * _849;
   uint16_t _851 = _depthwise_conv_stencil(5, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _852 = _depthwise_conv_s1_x + 1;
   uint16_t _853 = _input_host_global_wrapper_global_wrapper_stencil(5, _852, _depthwise_conv_s1_y);
   uint16_t _854 = (uint16_t)(30);
   uint16_t _855 = _853 * _854;
   int32_t _856 = _depthwise_conv_s1_x + 2;
   uint16_t _857 = _input_host_global_wrapper_global_wrapper_stencil(5, _856, _depthwise_conv_s1_y);
   uint16_t _858 = _857 * _849;
   int32_t _859 = _depthwise_conv_s1_y + 1;
   uint16_t _860 = _input_host_global_wrapper_global_wrapper_stencil(5, _depthwise_conv_s1_x, _859);
   uint16_t _861 = _860 * _854;
   uint16_t _862 = _input_host_global_wrapper_global_wrapper_stencil(5, _852, _859);
   uint16_t _863 = (uint16_t)(37);
   uint16_t _864 = _862 * _863;
   uint16_t _865 = _input_host_global_wrapper_global_wrapper_stencil(5, _856, _859);
   uint16_t _866 = _865 * _854;
   int32_t _867 = _depthwise_conv_s1_y + 2;
   uint16_t _868 = _input_host_global_wrapper_global_wrapper_stencil(5, _depthwise_conv_s1_x, _867);
   uint16_t _869 = _868 * _849;
   uint16_t _870 = _input_host_global_wrapper_global_wrapper_stencil(5, _856, _867);
   uint16_t _871 = _870 * _849;
   uint16_t _872 = _input_host_global_wrapper_global_wrapper_stencil(5, _852, _867);
   uint16_t _873 = _872 * _854;
   uint16_t _874 = _871 + _873;
   uint16_t _875 = _869 + _874;
   uint16_t _876 = _866 + _875;
   uint16_t _877 = _864 + _876;
   uint16_t _878 = _861 + _877;
   uint16_t _879 = _858 + _878;
   uint16_t _880 = _855 + _879;
   uint16_t _881 = _851 + _880;
   uint16_t _882 = _850 + _881;
   _depthwise_conv_stencil(5, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _882;
   uint16_t _925 = _input_host_global_wrapper_global_wrapper_stencil(6, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   uint16_t _926 = (uint16_t)(24);
   uint16_t _927 = _925 * _926;
   uint16_t _928 = _depthwise_conv_stencil(6, _depthwise_conv_s1_x, _depthwise_conv_s1_y);
   int32_t _929 = _depthwise_conv_s1_x + 1;
   uint16_t _930 = _input_host_global_wrapper_global_wrapper_stencil(6, _929, _depthwise_conv_s1_y);
   uint16_t _931 = (uint16_t)(30);
   uint16_t _932 = _930 * _931;
   int32_t _933 = _depthwise_conv_s1_x + 2;
   uint16_t _934 = _input_host_global_wrapper_global_wrapper_stencil(6, _933, _depthwise_conv_s1_y);
   uint16_t _935 = _934 * _926;
   int32_t _936 = _depthwise_conv_s1_y + 1;
   uint16_t _937 = _input_host_global_wrapper_global_wrapper_stencil(6, _depthwise_conv_s1_x, _936);
   uint16_t _938 = _937 * _931;
   uint16_t _939 = _input_host_global_wrapper_global_wrapper_stencil(6, _929, _936);
   uint16_t _940 = (uint16_t)(37);
   uint16_t _941 = _939 * _940;
   uint16_t _942 = _input_host_global_wrapper_global_wrapper_stencil(6, _933, _936);
   uint16_t _943 = _942 * _931;
   int32_t _944 = _depthwise_conv_s1_y + 2;
   uint16_t _945 = _input_host_global_wrapper_global_wrapper_stencil(6, _depthwise_conv_s1_x, _944);
   uint16_t _946 = _945 * _926;
   uint16_t _947 = _input_host_global_wrapper_global_wrapper_stencil(6, _933, _944);
   uint16_t _948 = _947 * _926;
   uint16_t _949 = _input_host_global_wrapper_global_wrapper_stencil(6, _929, _944);
   uint16_t _950 = _949 * _931;
   uint16_t _951 = _948 + _950;
   uint16_t _952 = _946 + _951;
   uint16_t _953 = _943 + _952;
   uint16_t _954 = _941 + _953;
   uint16_t _955 = _938 + _954;
   uint16_t _956 = _935 + _955;
   uint16_t _957 = _932 + _956;
   uint16_t _958 = _928 + _957;
   uint16_t _959 = _927 + _958;
   _depthwise_conv_stencil(6, _depthwise_conv_s1_x, _depthwise_conv_s1_y) = _959;
  } // for depthwise_conv_s1_x
 } // for depthwise_conv_s1_y
 // consume depthwise_conv.stencil
 for (int hw_output_s0_y_y_cgra = 0; hw_output_s0_y_y_cgra < 114; hw_output_s0_y_y_cgra++)
 {
  for (int hw_output_s0_x_x_cgra = 0; hw_output_s0_x_x_cgra < 114; hw_output_s0_x_x_cgra++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _960 = _depthwise_conv_stencil(0, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(0, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _960;
   uint16_t _961 = _depthwise_conv_stencil(1, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(1, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _961;
   uint16_t _962 = _depthwise_conv_stencil(2, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(2, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _962;
   uint16_t _963 = _depthwise_conv_stencil(3, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(3, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _963;
   uint16_t _964 = _depthwise_conv_stencil(4, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(4, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _964;
   uint16_t _965 = _depthwise_conv_stencil(5, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(5, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _965;
   uint16_t _966 = _depthwise_conv_stencil(6, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra);
   _hw_output_stencil(6, _hw_output_s0_x_x_cgra, _hw_output_s0_y_y_cgra) = _966;
  } // for hw_output_s0_x_x_cgra
 } // for hw_output_s0_y_y_cgra
 // consume hw_output.stencil
 for (int hw_output_global_wrapper_s0_y_y_glb = 0; hw_output_global_wrapper_s0_y_y_glb < 114; hw_output_global_wrapper_s0_y_y_glb++)
 {
  for (int hw_output_global_wrapper_s0_x_x_glb = 0; hw_output_global_wrapper_s0_x_x_glb < 114; hw_output_global_wrapper_s0_x_x_glb++)
  {
#pragma HLS PIPELINE II=1
   uint16_t _967 = _hw_output_stencil(0, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(0, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _967;
   uint16_t _968 = _hw_output_stencil(1, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(1, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _968;
   uint16_t _969 = _hw_output_stencil(2, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(2, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _969;
   uint16_t _970 = _hw_output_stencil(3, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(3, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _970;
   uint16_t _971 = _hw_output_stencil(4, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(4, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _971;
   uint16_t _972 = _hw_output_stencil(5, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(5, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _972;
   uint16_t _973 = _hw_output_stencil(6, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb);
   _hw_output_global_wrapper_glb_stencil(6, _hw_output_global_wrapper_s0_x_x_glb, _hw_output_global_wrapper_s0_y_y_glb) = _973;
  } // for hw_output_global_wrapper_s0_x_x_glb
 } // for hw_output_global_wrapper_s0_y_y_glb
} // kernel hls_targetdepthwise_conv

