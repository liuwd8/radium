#!/usr/bin/env python3
from genericpath import isfile
import json
import shlex
import sys
import os
import argparse
import xml.dom.minidom
from xml.dom.minidom import getDOMImplementation

BASEDIR = os.getcwd()
sys.path.insert(1, os.path.join(BASEDIR, '../../tools/grit'))
sys.path.insert(2, os.path.join(BASEDIR, '../../tools/python'))

from grit import grd_reader
from grit import util


def GetXTBIds(options, defines, target_platform=None):
    res_tree = grd_reader.Parse(options.i,
                                debug=False,
                                defines=defines,
                                target_platform=target_platform)
    res_tree.SetOutputLanguage('en')
    res_tree.OnlyTheseTranslations([])
    res_tree.RunGatherers()

    translation_ids = {}
    for node in res_tree:
        if not node.IsTranslateable():
            continue

        for clique in node.GetCliques():
            if not clique.IsTranslateable():
                continue
            if not clique.GetMessage().GetRealContent():
                continue

            # Some explanation is in order here.  Note that we can have
            # many messages with the same ID.
            #
            # The way we work around this is to maintain a list of cliques
            # per message ID (in the UberClique) and select the "best" one
            # (the first one that has a description, or an arbitrary one
            # if there is no description) for inclusion in the XMB file.
            # The translations are all going to be the same for messages
            # with the same ID, although the way we replace placeholders
            # might be slightly different.
            id = clique.GetMessage().GetId()
            translation_ids[id] = clique.GetMessage().GetDescription()

    return translation_ids


def GenerateLocalXTB(options: dict, id_desc_maps: dict, local: str):
    translation_ids = list(id_desc_maps.keys())
    xtb_input = f'{options.xtb}/generated_resources_{local}.xtb'
    dom = xml.dom.minidom.parse(xtb_input)
    translations = dom.documentElement.getElementsByTagName("translation")

    template_file = f'{options.template_file}/radium_strings_{local}.xtb'
    template_dom = xml.dom.minidom.parse(template_file)
    template_dom_translation = template_dom.documentElement.getElementsByTagName(
        "translation")

    impl = getDOMImplementation()
    doc_type = impl.createDocumentType('translationbundle', '', '')
    newdoc = impl.createDocument(None, "translationbundle", doc_type)
    newdoc.documentElement.setAttribute('lang', local)
    newdoc.documentElement.appendChild(newdoc.createTextNode('\n'))

    translated_ids = set()
    for translation in template_dom_translation:
        id = translation.getAttribute("id")
        if not id in translation_ids:
            continue

        desc_comment = template_dom.createComment(f'{id_desc_maps[id]}')
        newdoc.documentElement.appendChild(desc_comment)
        newdoc.documentElement.appendChild(template_dom.createTextNode('\n'))

        translated_ids.add(id)
        newdoc.documentElement.appendChild(translation)
        newdoc.documentElement.appendChild(newdoc.createTextNode('\n'))

    for translation in translations:
        id = translation.getAttribute("id")
        if not id in translation_ids or id in translated_ids:
            continue

        desc_comment = template_dom.createComment(f'{id_desc_maps[id]}')
        newdoc.documentElement.appendChild(desc_comment)
        newdoc.documentElement.appendChild(template_dom.createTextNode('\n'))

        translated_ids.add(id)
        newdoc.documentElement.appendChild(translation)
        newdoc.documentElement.appendChild(newdoc.createTextNode('\n'))

    for id in translation_ids:
        if id in translated_ids:
            continue

        desc_comment = template_dom.createComment(f'{id_desc_maps[id]}')
        newdoc.documentElement.appendChild(desc_comment)
        newdoc.documentElement.appendChild(newdoc.createTextNode('\n'))

        untransled_id = template_dom.createComment(
            f'<translation id="{id}">{id_desc_maps[id]} </translation>')
        newdoc.documentElement.appendChild(untransled_id)
        newdoc.documentElement.appendChild(template_dom.createTextNode('\n'))

    out_file = f'{options.o}/radium_strings_{local}.xtb'
    with open(out_file, "w", encoding='utf-8') as f:
        f.write("""<?xml version="1.0" ?>
<!DOCTYPE translationbundle>
""")
        f.write(newdoc.documentElement.toxml())
        f.write('\n')


def Main():
    parser = argparse.ArgumentParser()
    # parser.add_argument('--folder', required=True)
    parser.add_argument('-i', required=True)
    parser.add_argument('--xtb', required=True)
    parser.add_argument('--template_file', required=True)
    parser.add_argument('-o', required=True)
    parser.add_argument("-D", action="append", dest="defines", default=[])
    parser.add_argument("-E", action="append", dest="build_env", default=[])
    parser.add_argument("-t", dest="target_platform", default=None)
    parser.add_argument('locals', nargs='+')
    options = parser.parse_args()

    defines = {}
    for define in options.defines:
        name, val = util.ParseDefine(define)
        defines[name] = val

    ids = GetXTBIds(options, defines, options.target_platform)
    for local in options.locals:
        GenerateLocalXTB(options, ids, local)

    envs = {}
    for string in options.build_env:
        key, value = str(string).split('=')
        envs[key] = value

    target_gen_dir = envs['target_gen_dir']
    out_file = f'{target_gen_dir}/generate_translations.stamp'
    with open(out_file, "w", encoding='utf-8') as f:
        f.write('\n')



if __name__ == '__main__':
    sys.exit(Main())
