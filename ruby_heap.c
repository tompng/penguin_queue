#include <ruby.h>

static ID id_priority, id_cmp, id_call;
#define RB_STR_BUF_CAT(rstr, cstr) rb_str_buf_cat((rstr), (cstr), sizeof(cstr)-1);
struct node {
  long index, serial;
  VALUE heap, priority, value;
};
VALUE node_class;
void node_mark(struct node *ptr){
  rb_gc_mark(ptr->heap);
  rb_gc_mark(ptr->priority);
  rb_gc_mark(ptr->value);
}
VALUE node_alloc_internal(long index, long serial, VALUE heap, VALUE priority, VALUE value){
  struct node *ptr = ALLOC(struct node);
  ptr->index = index;
  ptr->serial = serial;
  ptr->heap = heap;
  ptr->priority = priority;
  ptr->value = value;
  return Data_Wrap_Struct(node_class, node_mark, -1, ptr);
}
#define NODE_PREPARE(self, name) struct node *name;Data_Get_Struct(self, struct node, name);
VALUE node_pri(VALUE self){
  NODE_PREPARE(self, ptr);
  return ptr->priority;
}
VALUE node_val(VALUE self){
  NODE_PREPARE(self, ptr);
  return ptr->value;
}
VALUE node_val_set(VALUE self, VALUE val){
  NODE_PREPARE(self, ptr);
  return ptr->value = val;
}
VALUE node_inspect(VALUE self){
  NODE_PREPARE(self, nptr);
  VALUE str = rb_str_buf_new(0);
  rb_str_buf_append(str, rb_class_name(CLASS_OF(self)));
  RB_STR_BUF_CAT(str, "{priority: ");
  rb_str_buf_append(str, rb_inspect(nptr->priority));
  RB_STR_BUF_CAT(str, ", value: ");
  rb_str_buf_append(str, rb_inspect(nptr->value));
  RB_STR_BUF_CAT(str, "}");
  return str;
}

struct heap_data{
  long serial;
  VALUE heap, compare_by;
};

long compare(VALUE a, VALUE b){
  if(RB_FIXNUM_P(a)&&RB_FIXNUM_P(b))
    return (long)a > (long)b ? 1 : (long)a < (long)b ? -1 : 0;
  if(RB_FLOAT_TYPE_P(a)&&RB_FLOAT_TYPE_P(b)){
    double fa=RFLOAT_VALUE(a),fb=RFLOAT_VALUE(b);
    return fa>fb?1:fa<fb?-1:0;
  }
  if(RB_TYPE_P(a, T_STRING)&&RB_TYPE_P(b, T_STRING))
    return rb_str_cmp(a, b);
  return rb_fix2long(rb_funcall(a, id_cmp, 1, b));
}
long compare_sid(long a, long b){return a>b?1:a<b?-1:0;}
void heap_mark(struct heap_data *st){rb_gc_mark(st->heap);}
void heap_free(struct heap_data *st){free(st);}
VALUE heap_alloc(VALUE klass){
  struct heap_data *ptr=ALLOC(struct heap_data);
  ptr->serial = 0;
  ptr->heap = rb_ary_new_capa(1);
  rb_ary_push(ptr->heap, Qnil);
  if(rb_block_given_p()){
    ptr->compare_by = rb_block_proc();
  }else{
    ptr->compare_by = Qnil;
  }
  return Data_Wrap_Struct(klass, heap_mark, heap_free, ptr);
}

#define HEAP_PREPARE(self, name) struct heap_data *name;Data_Get_Struct(self, struct heap_data, name);

void heap_up(VALUE self, VALUE node){
  HEAP_PREPARE(self, ptr);
  RARRAY_PTR_USE(ptr->heap, heap, {
    NODE_PREPARE(node, nptr);
    long index = nptr->index;
    while(index > 1){
      long pindex = index/2;
      VALUE pnode = heap[pindex];
      NODE_PREPARE(pnode, pptr);
      long cmp = compare(pptr->priority, nptr->priority);
      if(!cmp)cmp=compare_sid(pptr->serial, nptr->serial);
      if(cmp<0)break;
      pptr->index = index;
      heap[index] = pnode;
      index = pindex;
    }
    nptr->index = index;
    heap[index] = node;
  });
}

void heap_down(VALUE self, VALUE node){
  HEAP_PREPARE(self, ptr);
  long length = RARRAY_LEN(ptr->heap);
  RARRAY_PTR_USE(ptr->heap, heap, {
    NODE_PREPARE(node, nptr);
    long index = nptr->index;
    while(2*index < length){
      long lindex = 2*index;
      VALUE lnode = heap[lindex];
      NODE_PREPARE(lnode, lptr);
      if(lindex+1 < length){
        VALUE rnode = heap[lindex+1];
        NODE_PREPARE(rnode, rptr);
        long cmp = compare(lptr->priority, rptr->priority);
        if(!cmp)cmp=compare_sid(lptr->serial, rptr->serial);
        if(cmp >= 0){
          lindex += 1;
          lnode = rnode;
          lptr = rptr;
        }
      }
      long cmp = compare(nptr->priority, lptr->priority);
      if(!cmp)cmp=compare_sid(nptr->serial, lptr->serial);
      if(cmp <= 0)break;
      lptr->index = index;
      heap[index] = lnode;
      index = lindex;
    }
    nptr->index = index;
    heap[index] = node;
  });
}

VALUE heap_remove_node(VALUE self, VALUE node){
  if(!rb_obj_is_kind_of(node, node_class))return Qnil;
  HEAP_PREPARE(self, ptr);
  NODE_PREPARE(node, nptr);
  long length = RARRAY_LEN(ptr->heap);
  if(nptr->index >= length || RARRAY_AREF(ptr->heap, nptr->index) != node)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    VALUE replace_node = rb_ary_pop(ptr->heap);
    if(replace_node == node)return Qnil;
    NODE_PREPARE(replace_node, rptr);
    heap[nptr->index] = replace_node;
    rptr->index = nptr->index;
    long cmp = compare(rptr->priority, nptr->priority);
    if(!cmp)cmp = compare_sid(rptr->serial, nptr->serial);
    if(cmp > 0){
      heap_down(nptr->heap, replace_node);
    }else{
      heap_up(nptr->heap, replace_node);
    }
  });
  return Qnil;
}

VALUE node_remove(VALUE self){
  NODE_PREPARE(self, nptr);
  heap_remove_node(nptr->heap, self);
  return Qnil;
}

VALUE node_update_priority(VALUE node, VALUE priority){
  NODE_PREPARE(node, nptr);
  HEAP_PREPARE(nptr->heap, ptr);
  VALUE priority_was = nptr->priority;
  nptr->priority = priority;
  long cmp = compare(priority, priority_was);
  if(cmp == 0)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    if(heap[nptr->index] != node)return Qnil;
  });
  if(cmp < 0){
    heap_up(nptr->heap, node);
  }else{
    heap_down(nptr->heap, node);
  }
  return Qnil;
}

VALUE heap_enq_vp(VALUE self, VALUE value, VALUE priority){
  HEAP_PREPARE(self, ptr);
  if(ptr->compare_by != Qnil){
    priority = rb_funcall(ptr->compare_by, id_call, 1, value);
  }
  long length = RARRAY_LEN(ptr->heap);
  VALUE node = node_alloc_internal(length, ptr->serial, self, priority, value);
  ptr->serial ++;
  rb_ary_push(ptr->heap, node);
  heap_up(self, node);
  return node;
}

#define OPTHASH_GIVEN_P(opts) \
    (argc > 0 && !NIL_P((opts) = rb_check_hash_type(argv[argc-1])) && (--argc, 1))
VALUE heap_enq(int argc, VALUE *argv, VALUE self){
  VALUE value;
  VALUE opts, priority, pri;
  if (OPTHASH_GIVEN_P(opts)) {
    ID keyword_ids[] = {id_priority};
  	rb_get_kwargs(opts, keyword_ids, 0, 1, &pri);
  }
  rb_scan_args(argc, argv, "1", &value);
    priority = pri==Qundef ? value : pri;
  return heap_enq_vp(self, value, priority);
}
VALUE heap_push(VALUE self, VALUE value){
  return heap_enq_vp(self, value, value);
}
VALUE heap_push_multiple(int argc, VALUE *argv, VALUE self){
  VALUE nodes = rb_ary_new_capa(argc);
  for(int i=0;i<argc;i++)rb_ary_push(nodes, heap_push(self, argv[i]));
  return nodes;
}

VALUE heap_first_node(VALUE self){
  HEAP_PREPARE(self, ptr);
  long length = RARRAY_LEN(ptr->heap);
  if(length == 1)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    return heap[1];
  });
}
VALUE heap_first(VALUE self){
  VALUE node = heap_first_node(self);
  if(node == Qnil)return Qnil;
  NODE_PREPARE(node, nptr);
  return nptr->value;
}
VALUE heap_first_with_priority(VALUE self){
  VALUE node = heap_first_node(self);
  if(node == Qnil)return Qnil;
  NODE_PREPARE(node, nptr);
  return rb_ary_new_from_args(2, nptr->value, nptr->priority);
}

VALUE heap_deq_node(VALUE self){
  HEAP_PREPARE(self, ptr);
  long length = RARRAY_LEN(ptr->heap);
  if(length == 1)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    VALUE first = heap[1];
    VALUE node = rb_ary_pop(ptr->heap);
    NODE_PREPARE(node, nptr);
    if(length > 1){
      nptr->index = 1;
      heap_down(self, node);
    }
    return first;
  });
}
VALUE heap_deq(int argc, VALUE *argv, VALUE self){
  if(argc == 0){
    VALUE node = heap_deq_node(self);
    if(node == Qnil)return Qnil;
    NODE_PREPARE(node, nptr);
    return nptr->value;
  }else{
    VALUE nv;
    rb_scan_args(argc, argv, "1", &nv);
    long n = NUM2LONG(nv);
    if(n<0)rb_raise(rb_eArgError, "negative array size");
    HEAP_PREPARE(self, ptr);
    long length = RARRAY_LEN(ptr->heap)-1;
    if(n>length)n=length;
    VALUE result = rb_ary_new_capa(n);
    for(int i=0;i<n;i++){
      VALUE node = heap_deq_node(self);
      NODE_PREPARE(node, nptr);
      rb_ary_push(result, nptr->value);
    }
    return result;
  }
}
VALUE heap_deq_with_priority(VALUE self){
  VALUE node = heap_deq_node(self);
  if(node == Qnil)return Qnil;
  NODE_PREPARE(node, nptr);
  return rb_ary_new_from_args(2, nptr->value, nptr->priority);
}

VALUE heap_size(VALUE self){
  HEAP_PREPARE(self, ptr);
  return LONG2FIX(RARRAY_LEN(ptr->heap)-1);
}
VALUE heap_is_empty(VALUE self){
  HEAP_PREPARE(self, ptr);
  return RARRAY_LEN(ptr->heap) == 1 ? Qtrue : Qfalse;
}
VALUE heap_inspect(VALUE self){
  VALUE str = rb_str_buf_new(0);
  rb_str_buf_append(str, rb_class_name(CLASS_OF(self)));
  RB_STR_BUF_CAT(str, "{size: ");
  rb_str_buf_append(str, rb_inspect(heap_size(self)));
  RB_STR_BUF_CAT(str, "}");
  return str;
}

void Init_ruby_heap(void){
  id_priority = rb_intern("priority");
  id_call = rb_intern("call");
  id_cmp = rb_intern("<=>");

  VALUE heap_class = rb_define_class("CExtHeap", rb_cObject);
  rb_define_alloc_func(heap_class, heap_alloc);
  rb_define_method(heap_class, "size", heap_size, 0);
  rb_define_method(heap_class, "empty?", heap_is_empty, 0);
  rb_define_method(heap_class, "inspect", heap_inspect, 0);
  rb_define_method(heap_class, "first", heap_first, 0);
  rb_define_method(heap_class, "first_node", heap_first_node, 0);
  rb_define_method(heap_class, "first_with_priority", heap_first_with_priority, 0);
  rb_define_method(heap_class, "to_s", heap_inspect, 0);
  rb_define_method(heap_class, "push", heap_push_multiple, -1);
  rb_define_method(heap_class, "<<", heap_push, 1);
  rb_define_method(heap_class, "enq", heap_enq, -1);
  rb_define_method(heap_class, "unshift", heap_push_multiple, -1);
  rb_define_method(heap_class, "pop", heap_deq, -1);
  rb_define_method(heap_class, "shift", heap_deq, -1);
  rb_define_method(heap_class, "deq", heap_deq, -1);
  rb_define_method(heap_class, "deq_with_priority", heap_deq_with_priority, 0);
  rb_define_method(heap_class, "delete", heap_remove_node, 1);
  rb_define_method(heap_class, "remove", heap_remove_node, 1);

  node_class = rb_define_class_under(heap_class, "Node", rb_cObject);
  rb_undef_alloc_func(node_class);
  rb_define_method(node_class, "priority", node_pri, 0);
  rb_define_method(node_class, "priority=", node_update_priority, 1);
  rb_define_method(node_class, "value", node_val, 0);
  rb_define_method(node_class, "value=", node_val_set, 1);
  rb_define_method(node_class, "inspect", node_inspect, 0);
  rb_define_method(node_class, "to_s", node_inspect, 0);
  rb_define_method(node_class, "remove", node_remove, 0);
}
