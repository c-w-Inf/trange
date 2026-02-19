# trange

## Introduction

This tool is used for translation. It substitutes texts in source text files with a json.

## Usage

It reads a json from `stdin` as configuration. A simple sample json is as follows:

```json
{
    "constants": {
        "json": "JavaScript对象表示法文件"
    },
    "translation": {
        "README.md": [
            {
                "org": "This tool is used for translation. It substitutes texts in source text files with a json.",
                "row": 5,
                "trans": "此工具用于翻译。它使用一个#_(json)替换文本源文件中的文本。"
            },
            {
                "org": "json",
                "row": 9,
                "col": 64,
                "trans": "#_(json)"
            }
        ]
    }
}
```
