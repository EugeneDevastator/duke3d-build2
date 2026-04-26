# gen.py
import sys, os, re

SCALAR_TO_C = {
    'int8':   'int8_t',
    'int16':  'int16_t',
    'int32':  'int32_t',
    'int64':  'int64_t',
    'uint8':  'uint8_t',
    'uint16': 'uint16_t',
    'uint32': 'uint32_t',
    'uint64': 'uint64_t',
    'float':  'float',
    'double': 'double',
    'char':   'char',
}

KNOWN_STORED = set(SCALAR_TO_C.keys()) | {'bytes', 'string'}

def c_type(t):
    return SCALAR_TO_C.get(t, t)

def parse(path):
    structs = []
    current = None
    with open(path) as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith('#'):
                continue
            if line.startswith('STRUCT '):
                name = line[7:].rstrip(':').strip()
                current = {'name': name, 'fields': []}
                structs.append(current)
                continue
            if current is None:
                continue
            # name  stype[N or counter_type]  [rtype [suffix]]
            # name  stype                     [rtype [suffix]]
            m = re.match(r'(\w+)\s+(\w+)(?:\[(\w+)\])?(?:\s+(\w+))?(?:\s+(\w+))?', line)
            if not m:
                print(f'WARN: cannot parse: {line}')
                continue
            fname, stype, arr, rtype, suffix = m.groups()
            is_fixed  = arr is not None and re.match(r'^\d+$', arr) is not None
            is_dynamic = arr is not None and not is_fixed
            is_struct  = stype not in KNOWN_STORED
            current['fields'].append({
                'name':    fname,
                'stype':   stype,
                'arr':     arr,          # None | literal int str | counter type str
                'fixed':   is_fixed,
                'dynamic': is_dynamic,
                'rtype':   rtype,
                'suffix':  suffix,
                'struct':  is_struct,
            })
    return structs

def conv_read(stype, rtype, suffix):
    s = f'{stype}_to_{rtype}'
    return s + (f'_{suffix}' if suffix else '')

def conv_write(stype, rtype, suffix):
    s = f'{rtype}_to_{stype}'
    return s + (f'_{suffix}' if suffix else '')

def runtime_c_type(f):
    if f['rtype']:  return c_type(f['rtype'])
    if f['struct']: return f['stype']
    return c_type(f['stype'])

def emit_h(structs, guard):
    L = []
    L.append(f'#ifndef {guard}')
    L.append(f'#define {guard}')
    L.append('#include <stdint.h>')
    L.append('#include <stdlib.h>')
    L.append('#include <string.h>')
    L.append('')
    for s in structs:
        L.append(f'typedef struct {s["name"]} {s["name"]};')
    L.append('')
    for s in structs:
        sname = s['name']
        L.append(f'struct {sname} {{')
        for f in s['fields']:
            rt = runtime_c_type(f)
            n  = f['name']
            if f['fixed']:
                ct = c_type(f['stype'])
                L.append(f'    {ct} {n}[{f["arr"]}];')
            elif f['dynamic']:
                L.append(f'    {c_type(f["arr"])} {n}_n;')
                if f['stype'] == 'string':
                    L.append(f'    char** {n};')
                else:
                    L.append(f'    {rt}* {n};')
            elif f['stype'] == 'bytes':
                L.append(f'    int32_t {n}_len;')
                L.append(f'    uint8_t* {n};')
            elif f['stype'] == 'string':
                L.append(f'    char* {n};')
            else:
                L.append(f'    {rt} {n};')
        L.append(f'}};')
        L.append('')
        L.append(f'int  {sname}_read (const char* buf, {sname}* out);')
        L.append(f'int  {sname}_write(char* buf, const {sname}* in);')
        L.append(f'void {sname}_free ({sname}* v);')
        L.append('')
    L.append(f'#endif')
    return '\n'.join(L)

def emit_read_field(f, L):
    n     = f['name']
    stype = f['stype']
    rtype = f['rtype']

    if f['fixed']:
        ct   = c_type(stype)
        size = f['arr']
        L.append(f'    memcpy(out->{n},buf+pos,sizeof({ct})*{size}); pos+=sizeof({ct})*{size};')
        return

    if f['struct']:
        if f['dynamic']:
            ct = c_type(f['arr'])
            L.append(f'    {{ {ct} cnt; memcpy(&cnt,buf+pos,sizeof({ct})); pos+=sizeof({ct});')
            L.append(f'    out->{n}_n=cnt; out->{n}=({stype}*)malloc(sizeof({stype})*cnt);')
            L.append(f'    for(int i=0;i<(int)cnt;i++){{ pos+={stype}_read(buf+pos,&out->{n}[i]); }} }}')
        else:
            L.append(f'    pos+={stype}_read(buf+pos,&out->{n});')
        return

    if stype == 'string':
        if f['dynamic']:
            ct = c_type(f['arr'])
            L.append(f'    {{ {ct} cnt; memcpy(&cnt,buf+pos,sizeof({ct})); pos+=sizeof({ct});')
            L.append(f'    out->{n}_n=cnt; out->{n}=(char**)malloc(sizeof(char*)*cnt);')
            L.append(f'    for(int i=0;i<(int)cnt;i++){{ int32_t sl; memcpy(&sl,buf+pos,sizeof(int32_t)); pos+=sizeof(int32_t);')
            L.append(f'    out->{n}[i]=(char*)malloc(sl+1); memcpy(out->{n}[i],buf+pos,sl); out->{n}[i][sl]=0; pos+=sl; }} }}')
        else:
            L.append(f'    {{ int32_t sl; memcpy(&sl,buf+pos,sizeof(int32_t)); pos+=sizeof(int32_t);')
            L.append(f'    out->{n}=(char*)malloc(sl+1); memcpy(out->{n},buf+pos,sl); out->{n}[sl]=0; pos+=sl; }}')
        return

    if stype == 'bytes':
        L.append(f'    {{ int32_t bl; memcpy(&bl,buf+pos,sizeof(int32_t)); pos+=sizeof(int32_t);')
        L.append(f'    out->{n}_len=bl; out->{n}=(uint8_t*)malloc(bl); memcpy(out->{n},buf+pos,bl); pos+=bl; }}')
        return

    st = c_type(stype)
    rt = c_type(rtype) if rtype else st
    if f['dynamic']:
        ct = c_type(f['arr'])
        L.append(f'    {{ {ct} cnt; memcpy(&cnt,buf+pos,sizeof({ct})); pos+=sizeof({ct});')
        L.append(f'    out->{n}_n=cnt; out->{n}=({rt}*)malloc(sizeof({rt})*cnt);')
        L.append(f'    for(int i=0;i<(int)cnt;i++){{ {st} raw; memcpy(&raw,buf+pos,sizeof({st})); pos+=sizeof({st});')
        if rtype:
            L.append(f'    out->{n}[i]={conv_read(stype,rtype,f["suffix"])}(raw); }} }}')
        else:
            L.append(f'    out->{n}[i]=raw; }} }}')
    else:
        L.append(f'    {{ {st} raw; memcpy(&raw,buf+pos,sizeof({st})); pos+=sizeof({st});')
        if rtype:
            L.append(f'    out->{n}={conv_read(stype,rtype,f["suffix"])}(raw); }}')
        else:
            L.append(f'    out->{n}=raw; }}')

def emit_write_field(f, L):
    n     = f['name']
    stype = f['stype']
    rtype = f['rtype']

    if f['fixed']:
        ct   = c_type(stype)
        size = f['arr']
        L.append(f'    memcpy(buf+pos,in->{n},sizeof({ct})*{size}); pos+=sizeof({ct})*{size};')
        return

    if f['struct']:
        if f['dynamic']:
            ct = c_type(f['arr'])
            L.append(f'    memcpy(buf+pos,&in->{n}_n,sizeof({ct})); pos+=sizeof({ct});')
            L.append(f'    for(int i=0;i<(int)in->{n}_n;i++){{ pos+={stype}_write(buf+pos,&in->{n}[i]); }}')
        else:
            L.append(f'    pos+={stype}_write(buf+pos,&in->{n});')
        return

    if stype == 'string':
        if f['dynamic']:
            ct = c_type(f['arr'])
            L.append(f'    memcpy(buf+pos,&in->{n}_n,sizeof({ct})); pos+=sizeof({ct});')
            L.append(f'    for(int i=0;i<(int)in->{n}_n;i++){{ int32_t sl=(int32_t)strlen(in->{n}[i]);')
            L.append(f'    memcpy(buf+pos,&sl,sizeof(int32_t)); pos+=sizeof(int32_t);')
            L.append(f'    memcpy(buf+pos,in->{n}[i],sl); pos+=sl; }}')
        else:
            L.append(f'    {{ int32_t sl=(int32_t)strlen(in->{n});')
            L.append(f'    memcpy(buf+pos,&sl,sizeof(int32_t)); pos+=sizeof(int32_t);')
            L.append(f'    memcpy(buf+pos,in->{n},sl); pos+=sl; }}')
        return

    if stype == 'bytes':
        L.append(f'    memcpy(buf+pos,&in->{n}_len,sizeof(int32_t)); pos+=sizeof(int32_t);')
        L.append(f'    memcpy(buf+pos,in->{n},in->{n}_len); pos+=in->{n}_len;')
        return

    st = c_type(stype)
    if f['dynamic']:
        ct = c_type(f['arr'])
        L.append(f'    memcpy(buf+pos,&in->{n}_n,sizeof({ct})); pos+=sizeof({ct});')
        L.append(f'    for(int i=0;i<(int)in->{n}_n;i++){{')
        if rtype:
            L.append(f'        {st} raw={conv_write(stype,rtype,f["suffix"])}(in->{n}[i]);')
        else:
            L.append(f'        {st} raw=in->{n}[i];')
        L.append(f'        memcpy(buf+pos,&raw,sizeof({st})); pos+=sizeof({st}); }}')
    else:
        if rtype:
            L.append(f'    {{ {st} raw={conv_write(stype,rtype,f["suffix"])}(in->{n}); memcpy(buf+pos,&raw,sizeof({st})); pos+=sizeof({st}); }}')
        else:
            L.append(f'    memcpy(buf+pos,&in->{n},sizeof({st})); pos+=sizeof({st});')

def emit_c(structs, hname):
    L = []
    L.append(f'#include "{hname}"')
    L.append('')
    for s in structs:
        sname = s['name']

        L.append(f'int {sname}_read(const char* buf, {sname}* out) {{')
        L.append(f'    int pos=0;')
        for f in s['fields']:
            emit_read_field(f, L)
        L.append(f'    return pos;')
        L.append(f'}}')
        L.append('')

        L.append(f'int {sname}_write(char* buf, const {sname}* in) {{')
        L.append(f'    int pos=0;')
        for f in s['fields']:
            emit_write_field(f, L)
        L.append(f'    return pos;')
        L.append(f'}}')
        L.append('')

        L.append(f'void {sname}_free({sname}* v) {{')
        for f in s['fields']:
            n = f['name']
            if f['fixed']:
                pass  # stack array, nothing to free
            elif f['struct']:
                if f['dynamic']:
                    L.append(f'    for(int i=0;i<(int)v->{n}_n;i++) {f["stype"]}_free(&v->{n}[i]);')
                    L.append(f'    free(v->{n}); v->{n}=0;')
                else:
                    L.append(f'    {f["stype"]}_free(&v->{n});')
            elif f['stype'] in ('string', 'bytes'):
                if f['dynamic'] and f['stype'] == 'string':
                    L.append(f'    for(int i=0;i<(int)v->{n}_n;i++) free(v->{n}[i]);')
                L.append(f'    free(v->{n}); v->{n}=0;')
            elif f['dynamic']:
                L.append(f'    free(v->{n}); v->{n}=0;')
        L.append(f'}}')
        L.append('')

    return '\n'.join(L)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('usage: gen.py <file.def>')
        sys.exit(1)
    path    = sys.argv[1]
    base    = os.path.splitext(os.path.basename(path))[0]
    hname   = base + '_gen.h'
    cname   = base + '_gen.c'
    guard   = hname.replace('.','_').upper()
    structs = parse(path)
    with open(hname,'w') as f: f.write(emit_h(structs, guard))
    with open(cname,'w') as f: f.write(emit_c(structs, hname))
    print(f'ok: {hname} {cname} ({len(structs)} structs)')
