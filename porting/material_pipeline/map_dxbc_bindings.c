/* Use the public vkd3d-shader API to bind each scanned D3D register explicitly.
 * Build against the same verified source headers and library used to compile.
 * JSON goes to stdout; compiler diagnostics go to stderr. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vkd3d_shader.h>

int main(int argc, char **argv)
{
    struct vkd3d_shader_scan_descriptor_info scan = {0};
    struct vkd3d_shader_compile_info info = {0};
    struct vkd3d_shader_interface_info interface = {0};
    struct vkd3d_shader_resource_binding *bindings = NULL;
    struct vkd3d_shader_code output = {0};
    unsigned int major, minor, base;
    char *messages = NULL, *end;
    void *bytes = NULL;
    FILE *file = NULL;
    long length;
    int result = 1, status;

    if (argc != 4) { fprintf(stderr, "usage: probe INPUT OUTPUT BINDING_BASE\n"); return 2; }
    errno = 0;
    unsigned long parsed_base = strtoul(argv[3], &end, 10);
    if (errno || !*argv[3] || *end || parsed_base > 4096) return 2;
    base = parsed_base;
    vkd3d_shader_get_version(&major, &minor);
    if (major != 2 || minor != 0) { fprintf(stderr, "requires verified vkd3d-shader 2.0\n"); return 2; }
    file = fopen(argv[1], "rb");
    if (!file || fseek(file, 0, SEEK_END) || (length = ftell(file)) < 32
            || length > 64 * 1024 * 1024 || fseek(file, 0, SEEK_SET)) goto done;
    if (!(bytes = malloc(length)) || fread(bytes, 1, length, file) != (size_t)length) goto done;
    fclose(file); file = NULL;
    if (memcmp(bytes, "DXBC", 4)) goto done;
    scan.type = VKD3D_SHADER_STRUCTURE_TYPE_SCAN_DESCRIPTOR_INFO;
    info.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
    info.next = &scan;
    info.source.code = bytes;
    info.source.size = length;
    info.source_type = VKD3D_SHADER_SOURCE_DXBC_TPF;
    info.target_type = VKD3D_SHADER_TARGET_SPIRV_BINARY;
    info.log_level = VKD3D_SHADER_LOG_WARNING;
    info.source_name = "verified-original.dxbc";
    status = vkd3d_shader_scan(&info, &messages);
    if (messages) { fputs(messages, stderr); vkd3d_shader_free_messages(messages); messages = NULL; }
    if (status < 0 || scan.descriptor_count > 4096) goto done;
    bindings = calloc(scan.descriptor_count ? scan.descriptor_count : 1, sizeof(*bindings));
    if (!bindings) goto done;
    for (unsigned int i = 0; i < scan.descriptor_count; ++i)
    {
        const struct vkd3d_shader_descriptor_info *d = &scan.descriptors[i];
        struct vkd3d_shader_resource_binding *b = &bindings[i];
        if (d->count != 1 || d->register_space || d->type > VKD3D_SHADER_DESCRIPTOR_TYPE_SAMPLER
                || (d->flags & VKD3D_SHADER_DESCRIPTOR_INFO_FLAG_UAV_COUNTER))
        { fprintf(stderr, "unsupported descriptor range/counter\n"); goto done; }
        for (unsigned int j = 0; j < i; ++j)
            if (bindings[j].type == d->type && bindings[j].register_index == d->register_index)
            { fprintf(stderr, "duplicate D3D register\n"); goto done; }
        b->type = d->type;
        b->register_space = d->register_space;
        b->register_index = d->register_index;
        b->shader_visibility = VKD3D_SHADER_VISIBILITY_ALL;
        b->flags = VKD3D_SHADER_BINDING_FLAG_BUFFER | VKD3D_SHADER_BINDING_FLAG_IMAGE;
        b->binding.set = 0;
        b->binding.binding = base + i;
        b->binding.count = 1;
    }
    interface.type = VKD3D_SHADER_STRUCTURE_TYPE_INTERFACE_INFO;
    interface.bindings = bindings;
    interface.binding_count = scan.descriptor_count;
    info.next = &interface;
    status = vkd3d_shader_compile(&info, &output, &messages);
    if (messages) { fputs(messages, stderr); vkd3d_shader_free_messages(messages); messages = NULL; }
    if (status < 0 || !output.code || !output.size) goto done;
    file = fopen(argv[2], "wb");
    if (!file || fwrite(output.code, 1, output.size, file) != output.size) goto done;
    if (fclose(file)) { file = NULL; goto done; } file = NULL;
    printf("{\"library_version\":\"%u.%u\",\"bindings\":[", major, minor);
    for (unsigned int i = 0; i < scan.descriptor_count; ++i)
    {
        const struct vkd3d_shader_descriptor_info *d = &scan.descriptors[i];
        printf("%s{\"type\":%u,\"register_space\":%u,\"register_index\":%u,"
                "\"resource_type\":%u,\"resource_data_type\":%u,\"flags\":%u,"
                "\"count\":%u,\"set\":0,\"binding\":%u}", i ? "," : "",
                d->type, d->register_space, d->register_index, d->resource_type,
                d->resource_data_type, d->flags, d->count, base + i);
    }
    puts("]}");
    result = 0;
done:
    if (result) fprintf(stderr, "descriptor scan/mapped compilation failed\n");
    if (file) fclose(file);
    vkd3d_shader_free_messages(messages);
    vkd3d_shader_free_shader_code(&output);
    vkd3d_shader_free_scan_descriptor_info(&scan);
    free(bindings);
    free(bytes);
    return result;
}
