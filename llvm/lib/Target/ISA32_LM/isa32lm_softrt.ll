; ModuleID = 'isa32lm_softrt.c'
source_filename = "isa32lm_softrt.c"
target datalayout = "e-m:e-p:32:32-Fi8-i64:64-v128:64:128-a:0:32-n32-S64"
target triple = "armv6-unknown-none-eabi"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define dso_local noundef i32 @__divsi3(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 {
entry:
  %spec.select17 = tail call i32 @llvm.abs.i32(i32 %a, i1 false)
  %ub.0 = tail call i32 @llvm.abs.i32(i32 %b, i1 false)
  %0 = xor i32 %b, %a
  %div = udiv i32 %spec.select17, %ub.0
  %sub9 = sub i32 0, %div
  %tobool8.not18 = icmp slt i32 %0, 0
  %cond = select i1 %tobool8.not18, i32 %sub9, i32 %div
  ret i32 %cond
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define dso_local range(i32 -2147483647, -2147483648) i32 @__modsi3(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 {
entry:
  %cmp = icmp slt i32 %a, 0
  %spec.select = tail call i32 @llvm.abs.i32(i32 %a, i1 false)
  %ub.0 = tail call i32 @llvm.abs.i32(i32 %b, i1 false)
  %0 = urem i32 %spec.select, %ub.0
  %sub9 = sub nsw i32 0, %0
  %cond = select i1 %cmp, i32 %sub9, i32 %0
  ret i32 %cond
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define dso_local noundef i32 @__umodsi3(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 {
entry:
  %0 = urem i32 %a, %b
  ret i32 %0
}

; Function Attrs: nofree norecurse nosync nounwind memory(none)
define dso_local i64 @__udivdi3(i64 noundef %a, i64 noundef %b) local_unnamed_addr #1 {
entry:
  %shr.i = lshr i64 %a, 32
  %conv.i = trunc nuw i64 %shr.i to i32
  %conv.i7 = trunc i64 %a to i32
  %shr.i8 = lshr i64 %b, 32
  %conv.i9 = trunc nuw i64 %shr.i8 to i32
  %conv.i10 = trunc i64 %b to i32
  br label %for.body.i

for.body.i:                                       ; preds = %if.end10.i, %entry
  %i.033.i = phi i32 [ 63, %entry ], [ %dec.i, %if.end10.i ]
  %ql.032.i = phi i32 [ 0, %entry ], [ %ql.1.i, %if.end10.i ]
  %qh.031.i = phi i32 [ 0, %entry ], [ %qh.1.i, %if.end10.i ]
  %rl.030.i = phi i32 [ 0, %entry ], [ %rl.1.i, %if.end10.i ]
  %rh.029.i = phi i32 [ 0, %entry ], [ %rh.1.i, %if.end10.i ]
  %cmp1.i = icmp samesign ugt i32 %i.033.i, 31
  %sub.i = add nsw i32 %i.033.i, -32
  %shr.i11 = lshr i32 %conv.i, %sub.i
  %shr2.i = lshr i32 %conv.i7, %i.033.i
  %cond.in.i = select i1 %cmp1.i, i32 %shr.i11, i32 %shr2.i
  %cond.i = and i32 %cond.in.i, 1
  %or.i.i = tail call i32 @llvm.fshl.i32(i32 %rh.029.i, i32 %rl.030.i, i32 1)
  %shl1.i.i = shl i32 %rl.030.i, 1
  %or.i = or disjoint i32 %cond.i, %shl1.i.i
  %cmp.not.i.i = icmp eq i32 %or.i.i, %conv.i9
  %cmp1.i.i = icmp ule i32 %or.i.i, %conv.i9
  %cmp2.i.i = icmp ult i32 %or.i, %conv.i10
  %retval.0.in.i.i = select i1 %cmp.not.i.i, i1 %cmp2.i.i, i1 %cmp1.i.i
  br i1 %retval.0.in.i.i, label %if.end10.i, label %if.then.i

if.then.i:                                        ; preds = %for.body.i
  %cond.neg.i.i = sext i1 %cmp2.i.i to i32
  %sub.i.i = sub i32 %or.i, %conv.i10
  %sub1.i.i = sub i32 %or.i.i, %conv.i9
  %sub2.i.i = add i32 %sub1.i.i, %cond.neg.i.i
  br i1 %cmp1.i, label %if.then5.i, label %if.else.i

if.then5.i:                                       ; preds = %if.then.i
  %shl.i = shl nuw i32 1, %sub.i
  %or7.i = or i32 %shl.i, %qh.031.i
  br label %if.end10.i

if.else.i:                                        ; preds = %if.then.i
  %shl8.i = shl nuw i32 1, %i.033.i
  %or9.i = or i32 %shl8.i, %ql.032.i
  br label %if.end10.i

if.end10.i:                                       ; preds = %if.else.i, %if.then5.i, %for.body.i
  %rh.1.i = phi i32 [ %or.i.i, %for.body.i ], [ %sub2.i.i, %if.then5.i ], [ %sub2.i.i, %if.else.i ]
  %rl.1.i = phi i32 [ %or.i, %for.body.i ], [ %sub.i.i, %if.then5.i ], [ %sub.i.i, %if.else.i ]
  %qh.1.i = phi i32 [ %qh.031.i, %for.body.i ], [ %or7.i, %if.then5.i ], [ %qh.031.i, %if.else.i ]
  %ql.1.i = phi i32 [ %ql.032.i, %for.body.i ], [ %ql.032.i, %if.then5.i ], [ %or9.i, %if.else.i ]
  %dec.i = add nsw i32 %i.033.i, -1
  %cmp.not.i = icmp eq i32 %i.033.i, 0
  br i1 %cmp.not.i, label %udivmod64_core.exit, label %for.body.i, !llvm.loop !10

udivmod64_core.exit:                              ; preds = %if.end10.i
  %conv.i12 = zext i32 %qh.1.i to i64
  %shl.i13 = shl nuw i64 %conv.i12, 32
  %conv1.i = zext i32 %ql.1.i to i64
  %or.i14 = or disjoint i64 %shl.i13, %conv1.i
  ret i64 %or.i14
}

; Function Attrs: nofree norecurse nosync nounwind memory(none)
define dso_local i64 @__umoddi3(i64 noundef %a, i64 noundef %b) local_unnamed_addr #1 {
entry:
  %shr.i = lshr i64 %a, 32
  %conv.i = trunc nuw i64 %shr.i to i32
  %conv.i7 = trunc i64 %a to i32
  %shr.i8 = lshr i64 %b, 32
  %conv.i9 = trunc nuw i64 %shr.i8 to i32
  %conv.i10 = trunc i64 %b to i32
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %i.033.i = phi i32 [ 63, %entry ], [ %dec.i, %for.body.i ]
  %rl.030.i = phi i32 [ 0, %entry ], [ %rl.1.i, %for.body.i ]
  %rh.029.i = phi i32 [ 0, %entry ], [ %rh.1.i, %for.body.i ]
  %cmp1.i = icmp samesign ugt i32 %i.033.i, 31
  %sub.i = add nsw i32 %i.033.i, -32
  %shr.i11 = lshr i32 %conv.i, %sub.i
  %shr2.i = lshr i32 %conv.i7, %i.033.i
  %cond.in.i = select i1 %cmp1.i, i32 %shr.i11, i32 %shr2.i
  %cond.i = and i32 %cond.in.i, 1
  %or.i.i = tail call i32 @llvm.fshl.i32(i32 %rh.029.i, i32 %rl.030.i, i32 1)
  %shl1.i.i = shl i32 %rl.030.i, 1
  %or.i = or disjoint i32 %cond.i, %shl1.i.i
  %cmp.not.i.i = icmp eq i32 %or.i.i, %conv.i9
  %cmp1.i.i = icmp ule i32 %or.i.i, %conv.i9
  %cmp2.i.i = icmp ult i32 %or.i, %conv.i10
  %retval.0.in.i.i = select i1 %cmp.not.i.i, i1 %cmp2.i.i, i1 %cmp1.i.i
  %cond.neg.i.i = sext i1 %cmp2.i.i to i32
  %sub1.i.i = sub i32 %or.i.i, %conv.i9
  %sub2.i.i = add i32 %sub1.i.i, %cond.neg.i.i
  %rh.1.i = select i1 %retval.0.in.i.i, i32 %or.i.i, i32 %sub2.i.i
  %sub.i.i = select i1 %retval.0.in.i.i, i32 0, i32 %conv.i10
  %rl.1.i = sub i32 %or.i, %sub.i.i
  %dec.i = add nsw i32 %i.033.i, -1
  %cmp.not.i = icmp eq i32 %i.033.i, 0
  br i1 %cmp.not.i, label %udivmod64_core.exit, label %for.body.i, !llvm.loop !10

udivmod64_core.exit:                              ; preds = %for.body.i
  %conv.i12 = zext i32 %rh.1.i to i64
  %shl.i13 = shl nuw i64 %conv.i12, 32
  %conv1.i = zext i32 %rl.1.i to i64
  %or.i14 = or disjoint i64 %shl.i13, %conv1.i
  ret i64 %or.i14
}

; Function Attrs: nofree norecurse nosync nounwind memory(none)
define dso_local i64 @__divdi3(i64 noundef %a, i64 noundef %b) local_unnamed_addr #1 {
entry:
  %shr.i = lshr i64 %a, 32
  %conv.i = trunc nuw i64 %shr.i to i32
  %conv.i18 = trunc i64 %a to i32
  %shr.i19 = lshr i64 %b, 32
  %conv.i20 = trunc nuw i64 %shr.i19 to i32
  %conv.i21 = trunc i64 %b to i32
  %cmp = icmp sgt i64 %a, -1
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %add.i = sub i32 0, %conv.i18
  %cmp.i = icmp eq i32 %conv.i18, 0
  %cond.i = zext i1 %cmp.i to i32
  %not1.i = xor i32 %conv.i, -1
  %add2.i = add nuw i32 %cond.i, %not1.i
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %a_hi.0 = phi i32 [ %conv.i, %entry ], [ %add2.i, %if.then ]
  %a_lo.0 = phi i32 [ %conv.i18, %entry ], [ %add.i, %if.then ]
  %neg.0 = phi i32 [ 0, %entry ], [ 1, %if.then ]
  %cmp4 = icmp slt i64 %b, 0
  br i1 %cmp4, label %if.then5, label %if.end9

if.then5:                                         ; preds = %if.end
  %add.i22 = sub i32 0, %conv.i21
  %cmp.i23 = icmp eq i32 %conv.i21, 0
  %cond.i24 = zext i1 %cmp.i23 to i32
  %not1.i25 = xor i32 %conv.i20, -1
  %add2.i26 = add nuw i32 %cond.i24, %not1.i25
  %lnot.ext8 = zext i1 %cmp to i32
  br label %if.end9

if.end9:                                          ; preds = %if.then5, %if.end
  %b_hi.0 = phi i32 [ %add2.i26, %if.then5 ], [ %conv.i20, %if.end ]
  %b_lo.0 = phi i32 [ %add.i22, %if.then5 ], [ %conv.i21, %if.end ]
  %neg.1 = phi i32 [ %lnot.ext8, %if.then5 ], [ %neg.0, %if.end ]
  br label %for.body.i

for.body.i:                                       ; preds = %if.end10.i, %if.end9
  %i.033.i = phi i32 [ 63, %if.end9 ], [ %dec.i, %if.end10.i ]
  %ql.032.i = phi i32 [ 0, %if.end9 ], [ %ql.1.i, %if.end10.i ]
  %qh.031.i = phi i32 [ 0, %if.end9 ], [ %qh.1.i, %if.end10.i ]
  %rl.030.i = phi i32 [ 0, %if.end9 ], [ %rl.1.i, %if.end10.i ]
  %rh.029.i = phi i32 [ 0, %if.end9 ], [ %rh.1.i, %if.end10.i ]
  %cmp1.i = icmp samesign ugt i32 %i.033.i, 31
  %sub.i = add nsw i32 %i.033.i, -32
  %shr.i27 = lshr i32 %a_hi.0, %sub.i
  %shr2.i = lshr i32 %a_lo.0, %i.033.i
  %cond.in.i = select i1 %cmp1.i, i32 %shr.i27, i32 %shr2.i
  %cond.i28 = and i32 %cond.in.i, 1
  %or.i.i = tail call i32 @llvm.fshl.i32(i32 %rh.029.i, i32 %rl.030.i, i32 1)
  %shl1.i.i = shl i32 %rl.030.i, 1
  %or.i = or disjoint i32 %cond.i28, %shl1.i.i
  %cmp.not.i.i = icmp eq i32 %or.i.i, %b_hi.0
  %cmp1.i.i = icmp ule i32 %or.i.i, %b_hi.0
  %cmp2.i.i = icmp ult i32 %or.i, %b_lo.0
  %retval.0.in.i.i = select i1 %cmp.not.i.i, i1 %cmp2.i.i, i1 %cmp1.i.i
  br i1 %retval.0.in.i.i, label %if.end10.i, label %if.then.i

if.then.i:                                        ; preds = %for.body.i
  %cond.neg.i.i = sext i1 %cmp2.i.i to i32
  %sub.i.i = sub i32 %or.i, %b_lo.0
  %sub1.i.i = sub i32 %or.i.i, %b_hi.0
  %sub2.i.i = add i32 %sub1.i.i, %cond.neg.i.i
  br i1 %cmp1.i, label %if.then5.i, label %if.else.i

if.then5.i:                                       ; preds = %if.then.i
  %shl.i = shl nuw i32 1, %sub.i
  %or7.i = or i32 %shl.i, %qh.031.i
  br label %if.end10.i

if.else.i:                                        ; preds = %if.then.i
  %shl8.i = shl nuw i32 1, %i.033.i
  %or9.i = or i32 %shl8.i, %ql.032.i
  br label %if.end10.i

if.end10.i:                                       ; preds = %if.else.i, %if.then5.i, %for.body.i
  %rh.1.i = phi i32 [ %or.i.i, %for.body.i ], [ %sub2.i.i, %if.then5.i ], [ %sub2.i.i, %if.else.i ]
  %rl.1.i = phi i32 [ %or.i, %for.body.i ], [ %sub.i.i, %if.then5.i ], [ %sub.i.i, %if.else.i ]
  %qh.1.i = phi i32 [ %qh.031.i, %for.body.i ], [ %or7.i, %if.then5.i ], [ %qh.031.i, %if.else.i ]
  %ql.1.i = phi i32 [ %ql.032.i, %for.body.i ], [ %ql.032.i, %if.then5.i ], [ %or9.i, %if.else.i ]
  %dec.i = add nsw i32 %i.033.i, -1
  %cmp.not.i = icmp eq i32 %i.033.i, 0
  br i1 %cmp.not.i, label %udivmod64_core.exit, label %for.body.i, !llvm.loop !10

udivmod64_core.exit:                              ; preds = %if.end10.i
  %tobool10.not = icmp eq i32 %neg.1, 0
  br i1 %tobool10.not, label %if.end12, label %if.then11

if.then11:                                        ; preds = %udivmod64_core.exit
  %add.i29 = sub i32 0, %ql.1.i
  %cmp.i30 = icmp eq i32 %ql.1.i, 0
  %cond.i31 = zext i1 %cmp.i30 to i32
  %not1.i32 = xor i32 %qh.1.i, -1
  %add2.i33 = add i32 %cond.i31, %not1.i32
  br label %if.end12

if.end12:                                         ; preds = %if.then11, %udivmod64_core.exit
  %q_hi.0 = phi i32 [ %qh.1.i, %udivmod64_core.exit ], [ %add2.i33, %if.then11 ]
  %q_lo.0 = phi i32 [ %ql.1.i, %udivmod64_core.exit ], [ %add.i29, %if.then11 ]
  %conv.i34 = zext i32 %q_hi.0 to i64
  %shl.i35 = shl nuw i64 %conv.i34, 32
  %conv1.i = zext i32 %q_lo.0 to i64
  %or.i36 = or disjoint i64 %shl.i35, %conv1.i
  ret i64 %or.i36
}

; Function Attrs: nofree norecurse nosync nounwind memory(none)
define dso_local i64 @__moddi3(i64 noundef %a, i64 noundef %b) local_unnamed_addr #1 {
entry:
  %shr.i = lshr i64 %a, 32
  %conv.i = trunc nuw i64 %shr.i to i32
  %conv.i12 = trunc i64 %a to i32
  %shr.i13 = lshr i64 %b, 32
  %conv.i14 = trunc nuw i64 %shr.i13 to i32
  %conv.i15 = trunc i64 %b to i32
  %cmp = icmp sgt i64 %a, -1
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %add.i = sub i32 0, %conv.i12
  %cmp.i = icmp eq i32 %conv.i12, 0
  %cond.i = zext i1 %cmp.i to i32
  %not1.i = xor i32 %conv.i, -1
  %add2.i = add nuw i32 %cond.i, %not1.i
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %a_hi.0 = phi i32 [ %conv.i, %entry ], [ %add2.i, %if.then ]
  %a_lo.0 = phi i32 [ %conv.i12, %entry ], [ %add.i, %if.then ]
  %cmp4 = icmp slt i64 %b, 0
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.end
  %add.i16 = sub i32 0, %conv.i15
  %cmp.i17 = icmp eq i32 %conv.i15, 0
  %cond.i18 = zext i1 %cmp.i17 to i32
  %not1.i19 = xor i32 %conv.i14, -1
  %add2.i20 = add nuw i32 %cond.i18, %not1.i19
  br label %if.end6

if.end6:                                          ; preds = %if.then5, %if.end
  %b_hi.0 = phi i32 [ %add2.i20, %if.then5 ], [ %conv.i14, %if.end ]
  %b_lo.0 = phi i32 [ %add.i16, %if.then5 ], [ %conv.i15, %if.end ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %if.end6
  %i.033.i = phi i32 [ 63, %if.end6 ], [ %dec.i, %for.body.i ]
  %rl.030.i = phi i32 [ 0, %if.end6 ], [ %rl.1.i, %for.body.i ]
  %rh.029.i = phi i32 [ 0, %if.end6 ], [ %rh.1.i, %for.body.i ]
  %cmp1.i = icmp samesign ugt i32 %i.033.i, 31
  %sub.i = add nsw i32 %i.033.i, -32
  %shr.i21 = lshr i32 %a_hi.0, %sub.i
  %shr2.i = lshr i32 %a_lo.0, %i.033.i
  %cond.in.i = select i1 %cmp1.i, i32 %shr.i21, i32 %shr2.i
  %cond.i22 = and i32 %cond.in.i, 1
  %or.i.i = tail call i32 @llvm.fshl.i32(i32 %rh.029.i, i32 %rl.030.i, i32 1)
  %shl1.i.i = shl i32 %rl.030.i, 1
  %or.i = or disjoint i32 %cond.i22, %shl1.i.i
  %cmp.not.i.i = icmp eq i32 %or.i.i, %b_hi.0
  %cmp1.i.i = icmp ule i32 %or.i.i, %b_hi.0
  %cmp2.i.i = icmp ult i32 %or.i, %b_lo.0
  %retval.0.in.i.i = select i1 %cmp.not.i.i, i1 %cmp2.i.i, i1 %cmp1.i.i
  %cond.neg.i.i = sext i1 %cmp2.i.i to i32
  %sub1.i.i = sub i32 %or.i.i, %b_hi.0
  %sub2.i.i = add i32 %sub1.i.i, %cond.neg.i.i
  %rh.1.i = select i1 %retval.0.in.i.i, i32 %or.i.i, i32 %sub2.i.i
  %sub.i.i = select i1 %retval.0.in.i.i, i32 0, i32 %b_lo.0
  %rl.1.i = sub i32 %or.i, %sub.i.i
  %dec.i = add nsw i32 %i.033.i, -1
  %cmp.not.i = icmp eq i32 %i.033.i, 0
  br i1 %cmp.not.i, label %udivmod64_core.exit, label %for.body.i, !llvm.loop !10

udivmod64_core.exit:                              ; preds = %for.body.i
  br i1 %cmp, label %if.end8, label %if.then7

if.then7:                                         ; preds = %udivmod64_core.exit
  %add.i23 = sub i32 0, %rl.1.i
  %cmp.i24 = icmp eq i32 %rl.1.i, 0
  %cond.i25 = zext i1 %cmp.i24 to i32
  %not1.i26 = xor i32 %rh.1.i, -1
  %add2.i27 = add i32 %cond.i25, %not1.i26
  br label %if.end8

if.end8:                                          ; preds = %if.then7, %udivmod64_core.exit
  %r_hi.0 = phi i32 [ %rh.1.i, %udivmod64_core.exit ], [ %add2.i27, %if.then7 ]
  %r_lo.0 = phi i32 [ %rl.1.i, %udivmod64_core.exit ], [ %add.i23, %if.then7 ]
  %conv.i28 = zext i32 %r_hi.0 to i64
  %shl.i29 = shl nuw i64 %conv.i28, 32
  %conv1.i = zext i32 %r_lo.0 to i64
  %or.i30 = or disjoint i64 %shl.i29, %conv1.i
  ret i64 %or.i30
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define dso_local i64 @__muldi3(i64 noundef %a, i64 noundef %b) local_unnamed_addr #0 {
entry:
  %shr.i = lshr i64 %a, 32
  %conv.i = trunc nuw i64 %shr.i to i32
  %conv.i14 = trunc i64 %a to i32
  %shr.i15 = lshr i64 %b, 32
  %conv.i16 = trunc nuw i64 %shr.i15 to i32
  %conv.i17 = trunc i64 %b to i32
  %and.i = and i32 %conv.i14, 65535
  %shr.i18 = lshr i32 %conv.i14, 16
  %and1.i = and i32 %conv.i17, 65535
  %shr2.i = lshr i32 %conv.i17, 16
  %mul.i = mul nuw i32 %and1.i, %and.i
  %mul3.i = mul nuw i32 %shr2.i, %and.i
  %mul4.i = mul nuw i32 %and1.i, %shr.i18
  %mul5.i = mul nuw i32 %shr2.i, %shr.i18
  %add.i = add i32 %mul3.i, %mul4.i
  %cmp.i = icmp ult i32 %add.i, %mul3.i
  %shr6.i = lshr i32 %add.i, 16
  %shl.i = select i1 %cmp.i, i32 65536, i32 0
  %shl7.i = shl i32 %add.i, 16
  %add.i.i = add i32 %shl7.i, %mul.i
  %cmp.i.i = icmp ult i32 %add.i.i, %mul.i
  %cond.i.i = zext i1 %cmp.i.i to i32
  %mul = mul i32 %conv.i, %conv.i17
  %mul5 = mul i32 %conv.i16, %conv.i14
  %or.i = add i32 %mul5, %mul
  %add2.i.i = add i32 %or.i, %mul5.i
  %add1.i20.i = add i32 %add2.i.i, %shr6.i
  %add = add i32 %add1.i20.i, %shl.i
  %add1.i = add i32 %add, %cond.i.i
  %conv.i24 = zext i32 %add1.i to i64
  %shl.i25 = shl nuw i64 %conv.i24, 32
  %conv1.i = zext i32 %add.i.i to i64
  %or.i26 = or disjoint i64 %shl.i25, %conv1.i
  ret i64 %or.i26
}

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.fshl.i32(i32, i32, i32) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.abs.i32(i32, i1 immarg) #3

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="arm1136jf-s" "target-features"="+armv6,+dsp,+fp64,+strict-align,+vfp2,+vfp2sp,-aes,-d32,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fullfp16,-neon,-sha2,-thumb-mode,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" }
attributes #1 = { nofree norecurse nosync nounwind memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="arm1136jf-s" "target-features"="+armv6,+dsp,+fp64,+strict-align,+vfp2,+vfp2sp,-aes,-d32,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fullfp16,-neon,-sha2,-thumb-mode,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" }
attributes #2 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}
!llvm.errno.tbaa = !{!5}

!0 = !{i32 1, !"target-abi", !"aapcs"}
!1 = !{i32 1, !"min_enum_size", i32 4}
!2 = !{i32 4, !"arm-eabi-fp-denormal", i32 1}
!3 = !{i32 8, !"arm-eabi-fp-number-model", i32 3}
!4 = !{!"clang version 24.0.0git (https://github.com/llvm/llvm-project.git 48378de650fc590d905377ec09fddce008c69f73)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"__libc_errno", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = distinct !{!10, !11, !12}
!11 = !{!"llvm.loop.mustprogress"}
!12 = !{!"llvm.loop.unroll.disable"}
