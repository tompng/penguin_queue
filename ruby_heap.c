#include <ruby.h>

struct node {
  long index;
  VALUE heap, priority, value;
};
VALUE node_class;
void node_mark(struct node *ptr){
  rb_gc_mark(ptr->heap);
  rb_gc_mark(ptr->priority);
  rb_gc_mark(ptr->value);
}
VALUE node_alloc_internal(int index, VALUE heap, VALUE priority, VALUE value){
  struct node *ptr = ALLOC(struct node);
  ptr->index = index;
  ptr->heap = heap;
  ptr->priority = priority;
  ptr->value = value;
  return Data_Wrap_Struct(node_class, node_mark, -1, ptr);
}
long node_idx(VALUE self){
  struct node *ptr;
  Data_Get_Struct(self, struct node, ptr);
  return ptr->index;
}
void node_idx_set(VALUE self, long index){
  struct node *ptr;
  Data_Get_Struct(self, struct node, ptr);
  ptr->index = index;
}

VALUE node_pri(VALUE obj){
  struct node *ptr;
  Data_Get_Struct(obj, struct node, ptr);
  return ptr->priority;
}
VALUE node_val(VALUE obj){
  struct node *ptr;
  Data_Get_Struct(obj, struct node, ptr);
  return ptr->value;
}

struct heap_data{
  VALUE heap, compare_by;
};

ID id_cmp;
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

void heap_mark(struct heap_data *st){rb_gc_mark(st->heap);}
void heap_free(struct heap_data *st){free(st);}
VALUE heap_alloc(VALUE klass){
  struct heap_data *ptr=malloc(sizeof(struct heap_data));
  ptr->heap = rb_ary_new_capa(1);
  rb_ary_push(ptr->heap, Qnil);
  if(rb_block_given_p()){
    ptr->compare_by = rb_block_proc();
  }else{
    ptr->compare_by = Qnil;
  }
  return Data_Wrap_Struct(klass, heap_mark, heap_free, ptr);
}

#define HEAP_PREPARE(name) struct heap_data *name;Data_Get_Struct(self, struct heap_data, name);

void heap_up(VALUE self, VALUE node){
  HEAP_PREPARE(ptr);
  RARRAY_PTR_USE(ptr->heap, heap, {
    long index = node_idx(node);
    while(index > 1){
      long pindex = index/2;
      VALUE pnode = heap[pindex];
      int cmp = compare(node_pri(pnode), node_pri(node));
      if(cmp<0)break;
      heap[index] = pnode;
      node_idx_set(pnode, index);
      index = pindex;
    }
    node_idx_set(node, index);
    heap[index] = node;
  });
}

void heap_down(VALUE self, VALUE node){
  HEAP_PREPARE(ptr);
  long length = RARRAY_LEN(ptr->heap);
  RARRAY_PTR_USE(ptr->heap, heap, {
    long index = node_idx(node);
    while(2*index < length){
      long lindex = 2*index;
      VALUE lnode = heap[lindex];
      if(lindex+1 < length){
        VALUE rnode = heap[lindex+1];
        int cmp = compare(node_pri(lnode), node_pri(rnode));
        if(cmp >= 0){
          lindex += 1;
          lnode = rnode;
        }
      }
      int cmp = compare(node_pri(node), node_pri(lnode));
      if(cmp <= 0)break;
      node_idx_set(lnode, index);
      heap[index] = lnode;
      index = lindex;
    }
    node_idx_set(node, index);
    heap[index] = node;
  });
}

VALUE node_update_priority(VALUE node, VALUE priority){
  struct node *nptr;
  struct heap_data *hptr;
  Data_Get_Struct(node, struct node, nptr);
  Data_Get_Struct(nptr->heap, struct heap_data, hptr);
  VALUE priority_was = nptr->priority;
  nptr->priority = priority;
  int cmp = compare(priority, priority_was);
  if(cmp == 0)return Qnil;
  RARRAY_PTR_USE(hptr->heap, heap, {
    if(heap[nptr->index] != node)return Qnil;
  });
  if(cmp < 0){
    heap_up(nptr->heap, node);
  }else{
    heap_down(nptr->heap, node);
  }
  return Qnil;
}

ID id_call;
VALUE heap_enq_vp(VALUE self, VALUE value, VALUE priority){
  HEAP_PREPARE(ptr);
  if(ptr->compare_by != Qnil){
    priority = rb_funcall(ptr->compare_by, id_call, 1, value);
  }
  long length = RARRAY_LEN(ptr->heap);
  VALUE node = node_alloc_internal(length, self, priority, value);
  rb_ary_push(ptr->heap, node);
  heap_up(self, node);
  return node;
}

#define OPTHASH_GIVEN_P(opts) \
    (argc > 0 && !NIL_P((opts) = rb_check_hash_type(argv[argc-1])) && (--argc, 1))
static ID id_priority;
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

VALUE heap_deq_node(VALUE self){
  HEAP_PREPARE(ptr);
  long length = RARRAY_LEN(ptr->heap);
  if(length == 1)return Qnil;
  RARRAY_PTR_USE(ptr->heap, heap, {
    VALUE first = heap[1];
    VALUE node = rb_ary_pop(ptr->heap);
    if(length > 1){
      node_idx_set(node, 1);
      heap_down(self, node);
    }
    return first;
  });
}
VALUE heap_deq(VALUE self){
  VALUE node = heap_deq_node(self);
  if(node == Qnil)return Qnil;
  return node_val(node);
}
VALUE heap_deq_with_priority(VALUE self){
  VALUE node = heap_deq_node(self);
  if(node == Qnil)return Qnil;
  return rb_ary_new_from_args(2, node_val(node), node_pri(node));
}

VALUE heap_hoge(VALUE self){
  struct heap_data *ptr;
  Data_Get_Struct(self, struct heap_data, ptr);
  return ptr->heap;
}

void Init_ruby_heap(void){
  id_priority = rb_intern("priority");
  id_call = rb_intern("call");
  id_cmp = rb_intern("<=>");

  node_class = rb_define_class("CExtHeap::Node", rb_cObject);
  rb_define_method(node_class, "priority", node_pri, 0);
  rb_define_method(node_class, "priority=", node_update_priority, 1);
  rb_define_method(node_class, "value", node_val, 0);

  VALUE heap_class = rb_define_class("CExtHeap", rb_cObject);
  rb_define_const(heap_class, "Node", node_class);
  rb_define_alloc_func(heap_class, heap_alloc);
  rb_define_method(heap_class, "hoge", heap_hoge, 0);
  rb_define_method(heap_class, "push", heap_push_multiple, -1);
  rb_define_method(heap_class, "<<", heap_push, 1);
  rb_define_method(heap_class, "enq", heap_enq, -1);
  rb_define_method(heap_class, "unshift", heap_push_multiple, -1);
  rb_define_method(heap_class, "pop", heap_deq, 0);
  rb_define_method(heap_class, "deq", heap_deq, 0);
  rb_define_method(heap_class, "deq_with_priority", heap_deq_with_priority, 0);
  rb_define_method(heap_class, "shift", heap_deq, 0);
}
