#include <ruby.h>
typedef struct{
  VALUE heap;
}heap_struct;
ID id_cmp;
int compare(VALUE a, VALUE b){
  // if(RB_FIXNUM_P(a)&&RB_FIXNUM_P(b))
  //   return (long)a > (long)b ? 1 : (long)a < (long)b ? -1 : 0;
  // if(RB_FLOAT_TYPE_P(a)&&RB_FLOAT_TYPE_P(b)){
  //   double fa=RFLOAT_VALUE(a),fb=RFLOAT_VALUE(b);
  //   return fa>fb?1:fa<fb?-1:0;
  // }
  // if(RB_TYPE_P(a, T_STRING)&&RB_TYPE_P(b, T_STRING))
  //   return rb_str_cmp(a, b);
  return rb_fix2long(rb_funcall(a, id_cmp, 1, b));
}
void heap_mark(heap_struct *st){rb_gc_mark(st->heap);}
void heap_free(heap_struct *st){free(st);}
VALUE heap_alloc(VALUE klass){
  heap_struct *ptr=malloc(sizeof(heap_struct));
  ptr->heap = rb_ary_new();
  return Data_Wrap_Struct(klass, heap_mark, heap_free, ptr);
}
VALUE heap_push(VALUE self, VALUE value){
  heap_struct *ptr;
  Data_Get_Struct(self, heap_struct, ptr);
  long index = RARRAY_LEN(ptr->heap);
  rb_ary_push(ptr->heap, Qnil);
  RARRAY_PTR_USE(ptr->heap, heap_ptr, {
    while(index){
      long pindex = (index-1)/2;
      VALUE pvalue = heap_ptr[pindex];
      int cmp=compare(pvalue, value);
      if(cmp<0)break;
      heap_ptr[index] = pvalue;
      index = pindex;
    }
    heap_ptr[index] = value;
  });
  return self;
}
VALUE heap_pop(VALUE self){
  heap_struct *ptr;
  Data_Get_Struct(self, heap_struct, ptr);
  long length = RARRAY_LEN(ptr->heap);
  if(length == 0)return Qnil;
  long index = 0;
  VALUE value = rb_ary_pop(ptr->heap);
  length--;
  if(length == 0)return value;
  RARRAY_PTR_USE(ptr->heap, heap_ptr, {
    VALUE data = heap_ptr[0];
    while(1){
      long lindex = 2*index+1;
      long rindex = 2*index+2;
      if(lindex >= length)break;
      long cindex;
      VALUE cvalue;
      if(rindex >= length){
        cindex = lindex;
        cvalue = heap_ptr[lindex];
      }else{
        VALUE lvalue = heap_ptr[lindex];
        VALUE rvalue = heap_ptr[rindex];
        int cmp=compare(lvalue, rvalue);
        if(cmp<0){cindex = lindex; cvalue = lvalue;}
        else{cindex = rindex; cvalue = rvalue;}
      }
      int cmp=compare(value, cvalue);
      if(cmp <= 0)break;
      heap_ptr[index] = cvalue;
      index = cindex;
    }
    heap_ptr[index] = value;
    return data;
  });
}
VALUE heap_hoge(VALUE self){
  heap_struct *ptr;
  Data_Get_Struct(self, heap_struct, ptr);
  return ptr->heap;
}

void Init_ruby_heap(void){
  id_cmp = rb_intern("<=>");
  VALUE heap_class = rb_define_class("CExtHeap", rb_cObject);
  rb_define_alloc_func(heap_class, heap_alloc);
  rb_define_method(heap_class, "hoge", heap_hoge, 0);
  rb_define_method(heap_class, "push", heap_push, 1);
  rb_define_method(heap_class, "<<", heap_push, 1);
  rb_define_method(heap_class, "enq", heap_push, 1);
  rb_define_method(heap_class, "unshift", heap_push, 1);
  rb_define_method(heap_class, "pop", heap_pop, 0);
  rb_define_method(heap_class, "deq", heap_pop, 0);
  rb_define_method(heap_class, "shift", heap_pop, 0);
}
