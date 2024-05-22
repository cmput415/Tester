; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@sstate = internal global i32 undef

declare ptr @malloc(i64)

declare void @free(ptr)

declare void @print_integer(i32)

declare { i32, ptr } @real_to_string(float)

declare { i32, ptr } @int_to_string(i32)

define i32 @b_length(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @c_length(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @i_length(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @r_length(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @b_rows(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @c_rows(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @i_rows(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @r_rows(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  ret i32 %3
}

define i32 @b_columns(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  %4 = extractvalue { i32, ptr } %2, 0
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %10

6:                                                ; preds = %1
  %7 = extractvalue { i32, ptr } %2, 1
  %8 = load { i32, ptr }, ptr %7, align 8
  %9 = extractvalue { i32, ptr } %8, 0
  store i32 %9, ptr %3, align 4
  br label %10

10:                                               ; preds = %6, %1
  %11 = load i32, ptr %3, align 4
  ret i32 %11
}

define i32 @c_columns(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  %4 = extractvalue { i32, ptr } %2, 0
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %10

6:                                                ; preds = %1
  %7 = extractvalue { i32, ptr } %2, 1
  %8 = load { i32, ptr }, ptr %7, align 8
  %9 = extractvalue { i32, ptr } %8, 0
  store i32 %9, ptr %3, align 4
  br label %10

10:                                               ; preds = %6, %1
  %11 = load i32, ptr %3, align 4
  ret i32 %11
}

define i32 @i_columns(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  %4 = extractvalue { i32, ptr } %2, 0
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %10

6:                                                ; preds = %1
  %7 = extractvalue { i32, ptr } %2, 1
  %8 = load { i32, ptr }, ptr %7, align 8
  %9 = extractvalue { i32, ptr } %8, 0
  store i32 %9, ptr %3, align 4
  br label %10

10:                                               ; preds = %6, %1
  %11 = load i32, ptr %3, align 4
  ret i32 %11
}

define i32 @r_columns(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  %4 = extractvalue { i32, ptr } %2, 0
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %6, label %10

6:                                                ; preds = %1
  %7 = extractvalue { i32, ptr } %2, 1
  %8 = load { i32, ptr }, ptr %7, align 8
  %9 = extractvalue { i32, ptr } %8, 0
  store i32 %9, ptr %3, align 4
  br label %10

10:                                               ; preds = %6, %1
  %11 = load i32, ptr %3, align 4
  ret i32 %11
}

define { i32, ptr } @b_reverse(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  %4 = alloca { i32, ptr }, align 8
  %5 = zext i32 %3 to i64
  %6 = mul i64 %5, 8
  %7 = call ptr @malloc(i64 %6)
  %8 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 1
  store ptr %7, ptr %8, align 8
  %9 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 0
  store i32 %3, ptr %9, align 4
  %10 = load { i32, ptr }, ptr %4, align 8
  %11 = alloca i32, align 4
  %12 = sub i32 %3, 1
  store i32 %12, ptr %11, align 4
  %13 = alloca i32, align 4
  store i32 -1, ptr %13, align 4
  br label %15

14:                                               ; preds = %15
  ret { i32, ptr } %10

15:                                               ; preds = %19, %1
  %16 = load i32, ptr %13, align 4
  %17 = add i32 %16, 1
  store i32 %17, ptr %13, align 4
  %18 = icmp ult i32 %17, %3
  br i1 %18, label %19, label %14

19:                                               ; preds = %15
  %20 = load i32, ptr %13, align 4
  %21 = load i32, ptr %11, align 4
  %22 = extractvalue { i32, ptr } %2, 1
  %23 = getelementptr i1, ptr %22, i32 %21
  %24 = load i1, ptr %23, align 1
  %25 = extractvalue { i32, ptr } %10, 1
  %26 = getelementptr i1, ptr %25, i32 %20
  store i1 %24, ptr %26, align 1
  %27 = sub i32 %21, 1
  store i32 %27, ptr %11, align 4
  br label %15
}

define { i32, ptr } @c_reverse(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  %4 = alloca { i32, ptr }, align 8
  %5 = zext i32 %3 to i64
  %6 = mul i64 %5, 8
  %7 = call ptr @malloc(i64 %6)
  %8 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 1
  store ptr %7, ptr %8, align 8
  %9 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 0
  store i32 %3, ptr %9, align 4
  %10 = load { i32, ptr }, ptr %4, align 8
  %11 = alloca i32, align 4
  %12 = sub i32 %3, 1
  store i32 %12, ptr %11, align 4
  %13 = alloca i32, align 4
  store i32 -1, ptr %13, align 4
  br label %15

14:                                               ; preds = %15
  ret { i32, ptr } %10

15:                                               ; preds = %19, %1
  %16 = load i32, ptr %13, align 4
  %17 = add i32 %16, 1
  store i32 %17, ptr %13, align 4
  %18 = icmp ult i32 %17, %3
  br i1 %18, label %19, label %14

19:                                               ; preds = %15
  %20 = load i32, ptr %13, align 4
  %21 = load i32, ptr %11, align 4
  %22 = extractvalue { i32, ptr } %2, 1
  %23 = getelementptr i8, ptr %22, i32 %21
  %24 = load i8, ptr %23, align 1
  %25 = extractvalue { i32, ptr } %10, 1
  %26 = getelementptr i8, ptr %25, i32 %20
  store i8 %24, ptr %26, align 1
  %27 = sub i32 %21, 1
  store i32 %27, ptr %11, align 4
  br label %15
}

define { i32, ptr } @i_reverse(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  %4 = alloca { i32, ptr }, align 8
  %5 = zext i32 %3 to i64
  %6 = mul i64 %5, 32
  %7 = call ptr @malloc(i64 %6)
  %8 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 1
  store ptr %7, ptr %8, align 8
  %9 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 0
  store i32 %3, ptr %9, align 4
  %10 = load { i32, ptr }, ptr %4, align 8
  %11 = alloca i32, align 4
  %12 = sub i32 %3, 1
  store i32 %12, ptr %11, align 4
  %13 = alloca i32, align 4
  store i32 -1, ptr %13, align 4
  br label %15

14:                                               ; preds = %15
  ret { i32, ptr } %10

15:                                               ; preds = %19, %1
  %16 = load i32, ptr %13, align 4
  %17 = add i32 %16, 1
  store i32 %17, ptr %13, align 4
  %18 = icmp ult i32 %17, %3
  br i1 %18, label %19, label %14

19:                                               ; preds = %15
  %20 = load i32, ptr %13, align 4
  %21 = load i32, ptr %11, align 4
  %22 = extractvalue { i32, ptr } %2, 1
  %23 = getelementptr i32, ptr %22, i32 %21
  %24 = load i32, ptr %23, align 4
  %25 = extractvalue { i32, ptr } %10, 1
  %26 = getelementptr i32, ptr %25, i32 %20
  store i32 %24, ptr %26, align 4
  %27 = sub i32 %21, 1
  store i32 %27, ptr %11, align 4
  br label %15
}

define { i32, ptr } @r_reverse(ptr %0) {
  %2 = load { i32, ptr }, ptr %0, align 8
  %3 = extractvalue { i32, ptr } %2, 0
  %4 = alloca { i32, ptr }, align 8
  %5 = zext i32 %3 to i64
  %6 = mul i64 %5, 32
  %7 = call ptr @malloc(i64 %6)
  %8 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 1
  store ptr %7, ptr %8, align 8
  %9 = getelementptr { i32, ptr }, ptr %4, i32 0, i32 0
  store i32 %3, ptr %9, align 4
  %10 = load { i32, ptr }, ptr %4, align 8
  %11 = alloca i32, align 4
  %12 = sub i32 %3, 1
  store i32 %12, ptr %11, align 4
  %13 = alloca i32, align 4
  store i32 -1, ptr %13, align 4
  br label %15

14:                                               ; preds = %15
  ret { i32, ptr } %10

15:                                               ; preds = %19, %1
  %16 = load i32, ptr %13, align 4
  %17 = add i32 %16, 1
  store i32 %17, ptr %13, align 4
  %18 = icmp ult i32 %17, %3
  br i1 %18, label %19, label %14

19:                                               ; preds = %15
  %20 = load i32, ptr %13, align 4
  %21 = load i32, ptr %11, align 4
  %22 = extractvalue { i32, ptr } %2, 1
  %23 = getelementptr float, ptr %22, i32 %21
  %24 = load float, ptr %23, align 4
  %25 = extractvalue { i32, ptr } %10, 1
  %26 = getelementptr float, ptr %25, i32 %20
  store float %24, ptr %26, align 4
  %27 = sub i32 %21, 1
  store i32 %27, ptr %11, align 4
  br label %15
}

define { i32, ptr } @b_format(ptr %0) {
  %2 = load i1, ptr %0, align 1
  %3 = alloca i8, align 1
  store i8 70, ptr %3, align 1
  br i1 %2, label %4, label %5

4:                                                ; preds = %1
  store i8 84, ptr %3, align 1
  br label %5

5:                                                ; preds = %4, %1
  %6 = load i8, ptr %3, align 1
  %7 = alloca { i32, ptr }, align 8
  %8 = call ptr @malloc(i64 8)
  %9 = getelementptr { i32, ptr }, ptr %7, i32 0, i32 1
  store ptr %8, ptr %9, align 8
  %10 = getelementptr { i32, ptr }, ptr %7, i32 0, i32 0
  store i32 1, ptr %10, align 4
  %11 = load { i32, ptr }, ptr %7, align 8
  %12 = extractvalue { i32, ptr } %11, 1
  %13 = getelementptr i8, ptr %12, i32 0
  store i8 %6, ptr %13, align 1
  ret { i32, ptr } %11
}

define { i32, ptr } @c_format(ptr %0) {
  %2 = load i8, ptr %0, align 1
  %3 = alloca { i32, ptr }, align 8
  %4 = call ptr @malloc(i64 8)
  %5 = getelementptr { i32, ptr }, ptr %3, i32 0, i32 1
  store ptr %4, ptr %5, align 8
  %6 = getelementptr { i32, ptr }, ptr %3, i32 0, i32 0
  store i32 1, ptr %6, align 4
  %7 = load { i32, ptr }, ptr %3, align 8
  %8 = extractvalue { i32, ptr } %7, 1
  %9 = getelementptr i8, ptr %8, i32 0
  store i8 %2, ptr %9, align 1
  ret { i32, ptr } %7
}

define { i32, ptr } @i_format(ptr %0) {
  %2 = load i32, ptr %0, align 4
  %3 = call { i32, ptr } @int_to_string(i32 %2)
  ret { i32, ptr } %3
}

define { i32, ptr } @r_format(ptr %0) {
  %2 = load float, ptr %0, align 4
  %3 = call { i32, ptr } @real_to_string(float %2)
  ret { i32, ptr } %3
}

define i32 @main() {
  store i32 0, ptr @sstate, align 4
  call void @print_integer(i32 7)
  ret i32 0
}
