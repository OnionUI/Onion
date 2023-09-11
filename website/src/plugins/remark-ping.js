const unified = require('unified')
const remarkParse = require('remark-parse')

const remarkPing = require('remark-ping')

const plugin = (async () => {
    const stringify = await import('rehype-stringify')
    const remark2rehype = await import('remark-rehype')

    console.log(remarkPing);

    return unified()
        .use(remarkParse)
        .use(remarkPing, {
            pingUsername: (username) => true,
            userURL: (username) => `https://github.com/${username}`
        })
        .use(remark2rehype)
        .use(stringify);
})()

module.exports = plugin;
