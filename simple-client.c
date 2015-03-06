#include <glib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "mono/metadata/sgen-gc.h"
#include "mono/metadata/sgen-client.h"
#include "mono/metadata/sgen-pinning.h"
#include "mono/metadata/gc-internal-agnostic.h"
#include "mono/utils/hazard-pointer.h"

SgenThreadInfo main_thread_info;
pthread_key_t thread_info_key;
pthread_key_t small_id_key;

static void
register_small_id (void)
{
	g_assert (!pthread_getspecific (small_id_key));
	pthread_setspecific (small_id_key, (gpointer)(long)(mono_thread_small_id_alloc () + 1));
}

void
sgen_client_init (void)
{
	pthread_key_create (&thread_info_key, NULL);
	pthread_key_create (&small_id_key, NULL);

	register_small_id ();
}

size_t
sgen_client_vtable_get_instance_size (GCVTable *vtable)
{
	return sgen_client_slow_object_get_size (vtable, NULL);
}

static GCVTable array_fill_vtable;

GCVTable*
sgen_client_get_array_fill_vtable (void)
{
	static gboolean inited = FALSE;

	if (!inited) {
		gsize bitmap = 0;
		array_fill_vtable.descriptor = (mword)mono_gc_make_descr_for_array (TRUE, &bitmap, 0, 1);
		inited = TRUE;
	}

	return &array_fill_vtable;
}

gboolean
sgen_client_array_fill_range (char *start, size_t size)
{
	GCArray *array;

	if (size < sizeof (GCArray)) {
		memset (start, 0, size);
		return FALSE;
	}

	array = (GCArray*)start;
	array->vtable = sgen_client_get_array_fill_vtable ();
	array->size = size;

	return TRUE;
}

void
sgen_client_zero_array_fill_header (void *p, size_t size)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_object_is_array_fill (GCObject *o)
{
	return o->vtable == sgen_client_get_array_fill_vtable ();
}

void
sgen_client_object_register_finalizer_if_necessary (GCObject *obj)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_object_has_critical_finalizer (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_object_queued_for_finalization (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_run_finalize (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_finalize_notify (void)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_mark_ephemerons (ScanCopyContext ctx)
{
	return TRUE;
}

void
sgen_client_clear_unreachable_ephemerons (ScanCopyContext ctx)
{
}

gboolean
sgen_client_cardtable_scan_object (char *obj, mword block_obj_size, guint8 *cards, gboolean mod_union, SgenGrayQueue *queue)
{
	g_assert_not_reached ();
}

void
sgen_client_nursery_objects_pinned (void **definitely_pinned, int count)
{
}

void
sgen_client_collecting_minor (SgenPointerQueue *fin_ready_queue, SgenPointerQueue *critical_fin_queue)
{
}

void
sgen_client_collecting_major_1 (void)
{
}

void
sgen_client_pinned_los_object (char *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_collecting_major_2 (void)
{
}

void
sgen_client_collecting_major_3 (SgenPointerQueue *fin_ready_queue, SgenPointerQueue *critical_fin_queue)
{
}

void
sgen_client_degraded_allocation (size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_total_allocated_heap_changed (size_t allocated_heap)
{
}

void
sgen_client_out_of_memory (size_t size)
{
	g_assert_not_reached ();
}

const
char* sgen_client_description_for_internal_mem_type (int type)
{
	g_assert_not_reached ();
}

const char*
sgen_client_object_safe_name (GCObject *obj)
{
	g_assert_not_reached ();
}

const char*
sgen_client_vtable_get_namespace (GCVTable *vtable)
{
	g_assert_not_reached ();
}

const char*
sgen_client_vtable_get_name (GCVTable *vtable)
{
	g_assert_not_reached ();
}

void
sgen_client_pre_collection_checks (void)
{
}

size_t
sgen_client_page_size (void)
{
	return getpagesize ();
}

void*
sgen_client_valloc (size_t size, gboolean activate)
{
	char *p = mmap (NULL, size, activate ? PROT_READ | PROT_WRITE : PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
	g_assert (p);
	return p;
}

static char*
aligned_address (char *mem, size_t size, size_t alignment)
{
	char *aligned = (char*)((size_t)(mem + (alignment - 1)) & ~(alignment - 1));
	g_assert (aligned >= mem && aligned + size <= mem + size + alignment && !((size_t)aligned & (alignment - 1)));
	return aligned;
}

void*
sgen_client_valloc_aligned (size_t size, size_t alignment, gboolean activate)
{
	char *p = sgen_client_valloc (size + alignment, activate);
	char *aligned = aligned_address (p, size, alignment);
	if (aligned != p)
		sgen_client_vfree (p, aligned - p);
	if (aligned + size != p + size + alignment)
		sgen_client_vfree (aligned + size, p + alignment - aligned);
	return aligned;
}

void
sgen_client_mprotect (void *addr, size_t size, gboolean activate)
{
	int result = mprotect (addr, size, activate ? PROT_READ | PROT_WRITE : PROT_NONE);
	g_assert (!result);
}

void
sgen_client_vfree (void *addr, size_t size)
{
	munmap (addr, size);
}

void
sgen_client_thread_register (SgenThreadInfo* info, void *stack_bottom_fallback)
{
	g_assert (info == &main_thread_info);

	pthread_setspecific (thread_info_key, info);
	//pthread_setspecific (small_id_key, (gpointer)(long)(mono_thread_small_id_alloc () + 1));

	info->client_info.stack_bottom = stack_bottom_fallback;
}

void
sgen_client_thread_unregister (SgenThreadInfo *p)
{
	g_assert_not_reached ();
}

void
sgen_client_thread_register_worker (void)
{
	register_small_id ();
}

void
sgen_client_scan_thread_data (void *start_nursery, void *end_nursery, gboolean precise, SgenGrayQueue *queue)
{
	SgenThreadInfo *info;

	FOREACH_THREAD (info) {
		g_assert (info->client_info.stack_top < info->client_info.stack_bottom);

		sgen_conservatively_pin_objects_from (info->client_info.stack_top, info->client_info.stack_bottom, start_nursery, end_nursery, PIN_TYPE_STACK);
		sgen_conservatively_pin_objects_from ((void**)info->client_info.registers, (void**)(((char*)info->client_info.registers) + sizeof (info->client_info.registers)), start_nursery, end_nursery, PIN_TYPE_STACK);
	}
}

SgenThreadInfo*
mono_thread_info_current (void)
{
	return pthread_getspecific (thread_info_key);
}

int
mono_thread_info_get_small_id (void)
{
	gpointer value = pthread_getspecific (small_id_key);
	g_assert (value);
	return ((int)value) - 1;
}

void
sgen_client_stop_world (int generation)
{
	SgenThreadInfo *info = mono_thread_info_current ();
	void *dummy;

	g_assert (info == &main_thread_info);

	setjmp (info->client_info.registers);
	info->client_info.stack_top = &dummy;
}

void
sgen_client_restart_world (int generation, GGTimingInfo *timing)
{
}

gboolean
sgen_client_bridge_need_processing (void)
{
	return FALSE;
}

void
sgen_client_bridge_reset_data (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_processing_stw_step (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_wait_for_processing (void)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_processing_finish (int generation)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_bridge_is_bridge_object (GCObject *obj)
{
	g_assert_not_reached ();
}

void
sgen_client_bridge_register_finalized_object (GCObject *object)
{
	g_assert_not_reached ();
}

void
sgen_client_log_timing (GGTimingInfo *info, mword last_major_num_sections, mword last_los_memory_usage)
{
}

gboolean
sgen_client_handle_gc_param (const char *opt)
{
	g_assert_not_reached ();
}

void
sgen_client_print_gc_params_usage (void)
{
	g_assert_not_reached ();
}

gboolean
sgen_client_handle_gc_debug (const char *opt)
{
	g_assert_not_reached ();
}

void
sgen_client_print_gc_debug_usage (void)
{
	g_assert_not_reached ();
}

void
sgen_client_binary_protocol_collection_requested (int generation, size_t requested_size, gboolean force)
{
}

void
sgen_client_binary_protocol_collection_begin (int minor_gc_count, int generation)
{
	printf ("collecting gen %d - %d\n", generation, minor_gc_count);
}

void
sgen_client_binary_protocol_collection_end (int minor_gc_count, int generation, long long num_objects_scanned, long long num_unique_objects_scanned)
{
}

void
sgen_client_binary_protocol_concurrent_start (void)
{
}

void
sgen_client_binary_protocol_concurrent_update (void)
{
}

void
sgen_client_binary_protocol_concurrent_finish (void)
{
}

void
sgen_client_binary_protocol_sweep_begin (int generation, int full_sweep)
{
}

void
sgen_client_binary_protocol_sweep_end (int generation, int full_sweep)
{
}

void
sgen_client_binary_protocol_world_stopping (int generation, long long timestamp)
{
}

void
sgen_client_binary_protocol_world_stopped (int generation, long long timestamp, long long total_major_cards, long long marked_major_cards, long long total_los_cards, long long marked_los_cards)
{
}

void
sgen_client_binary_protocol_world_restarting (int generation, long long timestamp, long long total_major_cards, long long marked_major_cards, long long total_los_cards, long long marked_los_cards)
{
}

void
sgen_client_binary_protocol_world_restarted (int generation, long long timestamp)
{
}

void
sgen_client_binary_protocol_mark_start (int generation)
{
}

void
sgen_client_binary_protocol_mark_end (int generation)
{
}

void
sgen_client_binary_protocol_reclaim_start (int generation)
{
}

void
sgen_client_binary_protocol_reclaim_end (int generation)
{
}

void
sgen_client_binary_protocol_alloc (gpointer obj, gpointer vtable, size_t size)
{
}

void
sgen_client_binary_protocol_alloc_pinned (gpointer obj, gpointer vtable, size_t size)
{
}

void
sgen_client_binary_protocol_alloc_degraded (gpointer obj, gpointer vtable, size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_binary_protocol_pin (gpointer obj, gpointer vtable, size_t size)
{
}

void
sgen_client_binary_protocol_cement (gpointer ptr, gpointer vtable, size_t size)
{
	g_assert_not_reached ();
}

void
sgen_client_binary_protocol_copy (gpointer from, gpointer to, gpointer vtable, size_t size)
{
}

void
sgen_client_binary_protocol_global_remset (gpointer ptr, gpointer value, gpointer value_vtable)
{
}

void
sgen_client_binary_protocol_dislink_update (gpointer link, gpointer obj, gboolean track, gboolean staged)
{
	g_assert_not_reached ();
}

void
sgen_client_binary_protocol_empty (gpointer start, size_t size)
{
}

void
sgen_client_binary_protocol_thread_suspend (gpointer thread, gpointer stopped_ip)
{
}

void
sgen_client_binary_protocol_thread_restart (gpointer thread)
{
}

void
sgen_client_binary_protocol_thread_register (gpointer thread)
{
}

void
sgen_client_binary_protocol_thread_unregister (gpointer thread)
{
}

void
sgen_client_binary_protocol_missing_remset (gpointer obj, gpointer obj_vtable, int offset, gpointer value, gpointer value_vtable, gboolean value_pinned)
{
}

void
sgen_client_binary_protocol_cement_reset (void)
{
}

void
sgen_client_binary_protocol_domain_unload_begin (gpointer domain)
{
}

void
sgen_client_binary_protocol_domain_unload_end (gpointer domain)
{
}

void
sgen_client_counter_register_time (const char *name, guint64 *value, gboolean monotonic)
{
}

void
sgen_client_counter_register_int (const char *name, int *value)
{
}

void
sgen_client_counter_register_uint64 (const char *name, guint64 *value)
{
}

void
sgen_client_counter_register_byte_count (const char *name, mword *value, gboolean monotonic)
{
}
