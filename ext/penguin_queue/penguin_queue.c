#include <ruby.h>

static ID id_priority, id_cmp, id_call, id_max, id_min;
#define RB_STR_BUF_CAT(rstr, cstr) rb_str_buf_cat((rstr), (cstr), sizeof(cstr)-1);
struct node {
  long index, id;
  VALUE queue, priority, value;
};
struct queue_data{
  long counter;
  int compare_sgn;
  VALUE heap, compare_by;
};
#define NODE_DECLARE_PTR(name) struct node *name;
#define NODE_GET_PTR(self, name) Data_Get_Struct(self, struct node, name);
#define QUEUE_DECLARE_PTR(name) struct queue_data *name;
#define QUEUE_GET_PTR(self, name) Data_Get_Struct(self, struct queue_data, name);
#define NODE_PREPARE(self, name) NODE_DECLARE_PTR(name);NODE_GET_PTR(self, name);
#define QUEUE_PREPARE(self, name) QUEUE_DECLARE_PTR(name);QUEUE_GET_PTR(self, name);

VALUE queue_class, node_class;
void node_mark(struct node *ptr){
  rb_gc_mark(ptr->queue);
  rb_gc_mark(ptr->priority);
  rb_gc_mark(ptr->value);
}
VALUE node_alloc_internal(long index, long id, VALUE queue, VALUE priority, VALUE value){
  struct node *ptr = ALLOC(struct node);
  ptr->index = index;
  ptr->id = id;
  ptr->queue = queue;
  ptr->priority = priority;
  ptr->value = value;
  return Data_Wrap_Struct(node_class, node_mark, RUBY_DEFAULT_FREE, ptr);
}
VALUE node_pri(VALUE self){
  NODE_PREPARE(self, ptr);
  return ptr->priority;
}
VALUE node_val(VALUE self){
  NODE_PREPARE(self, ptr);
  return ptr->value;
}
VALUE node_inspect(VALUE self){
  VALUE str = rb_str_buf_new(0);
  NODE_PREPARE(self, nptr);
  rb_str_buf_append(str, rb_class_name(CLASS_OF(self)));
  RB_STR_BUF_CAT(str, "{priority: ");
  rb_str_buf_append(str, rb_inspect(nptr->priority));
  RB_STR_BUF_CAT(str, ", value: ");
  rb_str_buf_append(str, rb_inspect(nptr->value));
  RB_STR_BUF_CAT(str, "}");
  return str;
}

long compare(VALUE a, VALUE b){
#ifdef RB_FIXNUM_P
  if(RB_FIXNUM_P(a)&&RB_FIXNUM_P(b))
    return (long)a > (long)b ? 1 : (long)a < (long)b ? -1 : 0;
  if(RB_FLOAT_TYPE_P(a)&&RB_FLOAT_TYPE_P(b)){
    double fa=RFLOAT_VALUE(a),fb=RFLOAT_VALUE(b);
    return fa>fb?1:fa<fb?-1:0;
  }
  if(RB_TYPE_P(a, T_STRING)&&RB_TYPE_P(b, T_STRING))
    return rb_str_cmp(a, b);
#endif
  return rb_fix2long(rb_funcall(a, id_cmp, 1, b));
}

long compare_id(long a, long b){return a>b?1:a<b?-1:0;}

#define OPTHASH_GIVEN_P(opts) (argc > 0 && !NIL_P((opts) = rb_check_hash_type(argv[argc-1])) && (--argc, 1))

void queue_mark(struct queue_data *self){
  rb_gc_mark(self->heap);
  rb_gc_mark(self->compare_by);
}

VALUE queue_alloc(VALUE klass){
  struct queue_data *ptr=ALLOC(struct queue_data);
  ptr->counter = 0;
  ptr->compare_sgn = 1;
  ptr->heap = rb_ary_new_capa(1);
  rb_ary_push(ptr->heap, Qnil);
  if(rb_block_given_p()){
    ptr->compare_by = rb_block_proc();
  }else{
    ptr->compare_by = Qnil;
  }
  return Data_Wrap_Struct(klass, queue_mark, RUBY_DEFAULT_FREE, ptr);
}

VALUE queue_initialize(int argc, VALUE *argv, VALUE self){
  VALUE order;
  QUEUE_PREPARE(self, ptr);
  if(argc == 0)return self;
  rb_scan_args(argc, argv, "1", &order);
  if(order == ID2SYM(id_max)){
    ptr->compare_sgn = -1;
  }else if(order != ID2SYM(id_min)){
    rb_raise(rb_eArgError, "order should be :min or :max");
  }
  return self;
}

VALUE queue_clear(VALUE self){
  QUEUE_PREPARE(self, ptr);
  ptr->counter = 0;
  rb_ary_clear(ptr->heap);
  rb_ary_push(ptr->heap, Qnil);
  return self;
}

void queue_up(VALUE self, VALUE node){
  int sgn;
  QUEUE_PREPARE(self, ptr);
  sgn = ptr->compare_sgn;
  RARRAY_PTR_USE(ptr->heap, heap, {
    long index;
    NODE_PREPARE(node, nptr);
    index = nptr->index;
    while(index > 1){
      long cmp;
      long pindex = index/2;
      VALUE pnode = heap[pindex];
      NODE_PREPARE(pnode, pptr);
      cmp = compare(pptr->priority, nptr->priority)*sgn;
      if(!cmp)cmp=compare_id(pptr->id, nptr->id);
      if(cmp<0)break;
      pptr->index = index;
      heap[index] = pnode;
      index = pindex;
    }
    nptr->index = index;
    heap[index] = node;
  });
}

void queue_down(VALUE self, VALUE node){
  int sgn;
  long length;
  QUEUE_PREPARE(self, ptr);
  sgn = ptr->compare_sgn;
  length = RARRAY_LEN(ptr->heap);
  RARRAY_PTR_USE(ptr->heap, heap, {
    long index;
    NODE_PREPARE(node, nptr);
    index = nptr->index;
    while(2*index < length){
      long lindex = 2*index;
      VALUE lnode = heap[lindex];
      NODE_PREPARE(lnode, lptr);
      if(lindex+1 < length){
        long cmp;
        VALUE rnode = heap[lindex+1];
        NODE_PREPARE(rnode, rptr);
        cmp = compare(lptr->priority, rptr->priority)*sgn;
        if(!cmp)cmp=compare_id(lptr->id, rptr->id);
        if(cmp >= 0){
          lindex += 1;
          lnode = rnode;
          lptr = rptr;
        }
      }
      long cmp = compare(nptr->priority, lptr->priority)*sgn;
      if(!cmp)cmp=compare_id(nptr->id, lptr->id);
      if(cmp <= 0)break;
      lptr->index = index;
      heap[index] = lnode;
      index = lindex;
    }
    nptr->index = index;
    heap[index] = node;
  });
}

VALUE queue_remove_node(VALUE self, VALUE node){
  int sgn;
  long length;
  QUEUE_DECLARE_PTR(ptr);
  NODE_DECLARE_PTR(nptr);
  if(!rb_obj_is_kind_of(node, node_class))return Qnil;
  QUEUE_GET_PTR(self, ptr);
  NODE_GET_PTR(node, nptr);
  sgn = ptr->compare_sgn;
  length = RARRAY_LEN(ptr->heap);
  if(nptr->index >= length || RARRAY_AREF(ptr->heap, nptr->index) != node)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    long cmp;
    VALUE replace_node = rb_ary_pop(ptr->heap);
    NODE_PREPARE(replace_node, rptr);
    if(replace_node == node)return Qnil;
    heap[nptr->index] = replace_node;
    rptr->index = nptr->index;
    cmp = compare(rptr->priority, nptr->priority)*sgn;
    if(!cmp)cmp = compare_id(rptr->id, nptr->id);
    if(cmp > 0){
      queue_down(nptr->queue, replace_node);
    }else{
      queue_up(nptr->queue, replace_node);
    }
  });
  return Qnil;
}

VALUE node_remove(VALUE self){
  NODE_PREPARE(self, nptr);
  queue_remove_node(nptr->queue, self);
  return Qnil;
}

void node_update_priority_internal(VALUE node, VALUE priority){
  int sgn;
  long cmp;
  VALUE priority_was;
  NODE_DECLARE_PTR(nptr);
  QUEUE_DECLARE_PTR(ptr);
  NODE_GET_PTR(node, nptr);
  QUEUE_GET_PTR(nptr->queue, ptr);
  sgn = ptr->compare_sgn;
  priority_was = nptr->priority;
  nptr->priority = priority;
  cmp = compare(priority, priority_was)*sgn;
  if(cmp == 0)return;
  RARRAY_PTR_USE(ptr->heap, heap, {
    if(heap[nptr->index] != node)return;
  });
  if(cmp < 0){
    queue_up(nptr->queue, node);
  }else{
    queue_down(nptr->queue, node);
  }
}

VALUE node_update_priority(VALUE node, VALUE priority){
  NODE_DECLARE_PTR(nptr);
  QUEUE_DECLARE_PTR(ptr);
  NODE_GET_PTR(node, nptr);
  QUEUE_GET_PTR(nptr->queue, ptr);
  if(ptr->compare_by != Qnil)rb_raise(rb_eRuntimeError, "priority update not supported on queue initialized with block");
  node_update_priority_internal(node, priority);
  return priority;
}

VALUE node_update_value(VALUE node, VALUE value){
  NODE_DECLARE_PTR(nptr);
  QUEUE_DECLARE_PTR(ptr);
  NODE_GET_PTR(node, nptr);
  QUEUE_GET_PTR(nptr->queue, ptr);
  nptr->value = value;
  if(ptr->compare_by != Qnil){
    VALUE priority = rb_funcall(ptr->compare_by, id_call, 1, value);
    node_update_priority_internal(node, priority);
  }
  return value;
}

VALUE queue_enq_vp(VALUE self, VALUE value, VALUE priority){
  long length;
  VALUE node;
  QUEUE_PREPARE(self, ptr);
  if(ptr->compare_by != Qnil){
    priority = rb_funcall(ptr->compare_by, id_call, 1, value);
  }
  length = RARRAY_LEN(ptr->heap);
  node = node_alloc_internal(length, ptr->counter, self, priority, value);
  ptr->counter++;
  rb_ary_push(ptr->heap, node);
  queue_up(self, node);
  return node;
}

VALUE queue_enq(int argc, VALUE *argv, VALUE self){
  VALUE value, opts, priority, pri  = Qundef;
  if (OPTHASH_GIVEN_P(opts)) {
    ID keyword_ids[] = {id_priority};
  	rb_get_kwargs(opts, keyword_ids, 0, 1, &pri);
  }
  rb_scan_args(argc, argv, "1", &value);
  priority = (pri == Qundef) ? value : pri;
  return queue_enq_vp(self, value, priority);
}
VALUE queue_push(VALUE self, VALUE value){
  return queue_enq_vp(self, value, value);
}

VALUE queue_first_node(VALUE self){
  long length;
  QUEUE_PREPARE(self, ptr);
  length = RARRAY_LEN(ptr->heap);
  if(length == 1)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    return heap[1];
  });
}
VALUE queue_first(VALUE self){
  VALUE node = queue_first_node(self);
  NODE_DECLARE_PTR(nptr);
  if(node == Qnil)return Qnil;
  NODE_GET_PTR(node, nptr);
  return nptr->value;
}
VALUE queue_first_with_priority(VALUE self){
  VALUE node = queue_first_node(self);
  NODE_DECLARE_PTR(nptr);
  if(node == Qnil)return Qnil;
  NODE_GET_PTR(node, nptr);
  return rb_ary_new_from_args(2, nptr->value, nptr->priority);
}

VALUE queue_deq_node(VALUE self){
  long length;
  QUEUE_PREPARE(self, ptr);
  length = RARRAY_LEN(ptr->heap);
  if(length == 1)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    VALUE first = heap[1];
    VALUE node = rb_ary_pop(ptr->heap);
    NODE_PREPARE(node, nptr);
    if(length > 1){
      nptr->index = 1;
      queue_down(self, node);
    }
    return first;
  });
}
VALUE queue_deq(int argc, VALUE *argv, VALUE self){
  if(argc == 0){
    VALUE node = queue_deq_node(self);
    NODE_DECLARE_PTR(nptr);
    if(node == Qnil)return Qnil;
    NODE_GET_PTR(node, nptr);
    return nptr->value;
  }else{
    VALUE nv, result;
    int i;
    long n, length;
    QUEUE_DECLARE_PTR(ptr);
    rb_scan_args(argc, argv, "1", &nv);
    n = NUM2LONG(nv);
    if(n<0)rb_raise(rb_eArgError, "negative array size");
    QUEUE_GET_PTR(self, ptr);
    length = RARRAY_LEN(ptr->heap)-1;
    if(n>length)n=length;
    result = rb_ary_new_capa(n);
    for(i=0;i<n;i++){
      VALUE node = queue_deq_node(self);
      NODE_PREPARE(node, nptr);
      rb_ary_push(result, nptr->value);
    }
    return result;
  }
}
VALUE queue_deq_with_priority(VALUE self){
  VALUE node = queue_deq_node(self);
  NODE_DECLARE_PTR(nptr);
  if(node == Qnil)return Qnil;
  NODE_GET_PTR(node, nptr);
  return rb_ary_new_from_args(2, nptr->value, nptr->priority);
}

VALUE queue_size(VALUE self){
  QUEUE_PREPARE(self, ptr);
  return LONG2FIX(RARRAY_LEN(ptr->heap)-1);
}
VALUE queue_is_empty(VALUE self){
  QUEUE_PREPARE(self, ptr);
  return RARRAY_LEN(ptr->heap) == 1 ? Qtrue : Qfalse;
}
#define QUEUE_PTR_IS_MIN(ptr) ((ptr)->compare_sgn==1)
VALUE queue_is_min(VALUE self){
  QUEUE_PREPARE(self, ptr);
  return QUEUE_PTR_IS_MIN(ptr) ? Qtrue : Qfalse;
}
VALUE queue_is_max(VALUE self){
  QUEUE_PREPARE(self, ptr);
  return QUEUE_PTR_IS_MIN(ptr) ? Qfalse : Qtrue;
}
VALUE queue_inspect(VALUE self){
  VALUE str = rb_str_buf_new(0);
  QUEUE_PREPARE(self, ptr);
  rb_str_buf_append(str, rb_class_name(CLASS_OF(self)));
  RB_STR_BUF_CAT(str, "{order: ");
  if(QUEUE_PTR_IS_MIN(ptr)){
    RB_STR_BUF_CAT(str, ":min");
  }else{
    RB_STR_BUF_CAT(str, ":max");
  }
  RB_STR_BUF_CAT(str, ", size: ");
  rb_str_buf_append(str, rb_inspect(queue_size(self)));
  RB_STR_BUF_CAT(str, "}");
  return str;
}

void Init_penguin_queue(void){
  id_priority = rb_intern("priority");
  id_call = rb_intern("call");
  id_cmp = rb_intern("<=>");
  id_max = rb_intern("max");
  id_min = rb_intern("min");

  queue_class = rb_define_class("PenguinQueue", rb_cObject);
  rb_define_alloc_func(queue_class, queue_alloc);
  rb_define_method(queue_class, "initialize", queue_initialize, -1);
  rb_define_method(queue_class, "size", queue_size, 0);
  rb_define_method(queue_class, "empty?", queue_is_empty, 0);
  rb_define_method(queue_class, "clear", queue_clear, 0);
  rb_define_method(queue_class, "inspect", queue_inspect, 0);
  rb_define_method(queue_class, "top", queue_first, 0);
  rb_define_method(queue_class, "peek", queue_first, 0);
  rb_define_method(queue_class, "first", queue_first, 0);
  rb_define_method(queue_class, "first_node", queue_first_node, 0);
  rb_define_method(queue_class, "first_with_priority", queue_first_with_priority, 0);
  rb_define_method(queue_class, "to_s", queue_inspect, 0);
  rb_define_method(queue_class, "push", queue_enq, -1);
  rb_define_method(queue_class, "<<", queue_push, 1);
  rb_define_method(queue_class, "enq", queue_enq, -1);
  rb_define_method(queue_class, "unshift", queue_enq, -1);
  rb_define_method(queue_class, "pop", queue_deq, -1);
  rb_define_method(queue_class, "shift", queue_deq, -1);
  rb_define_method(queue_class, "deq", queue_deq, -1);
  rb_define_method(queue_class, "poll", queue_deq, -1);
  rb_define_method(queue_class, "deq_with_priority", queue_deq_with_priority, 0);
  rb_define_method(queue_class, "delete", queue_remove_node, 1);
  rb_define_method(queue_class, "remove", queue_remove_node, 1);
  rb_define_method(queue_class, "min?", queue_is_min, 0);
  rb_define_method(queue_class, "max?", queue_is_max, 0);

  node_class = rb_define_class_under(queue_class, "Node", rb_cObject);
  rb_undef_alloc_func(node_class);
  rb_define_method(node_class, "priority", node_pri, 0);
  rb_define_method(node_class, "priority=", node_update_priority, 1);
  rb_define_method(node_class, "value", node_val, 0);
  rb_define_method(node_class, "value=", node_update_value, 1);
  rb_define_method(node_class, "inspect", node_inspect, 0);
  rb_define_method(node_class, "to_s", node_inspect, 0);
  rb_define_method(node_class, "remove", node_remove, 0);
  rb_define_method(node_class, "delete", node_remove, 0);
}
