// Turns a paragraph that holds only a YouTube URL into an embedded video.
// Replaces remark-oembed, which empties every link-only paragraph when one
// of the providers fetched from oembed.com has a scheme it can't turn into
// a RegExp (e.g. "https://app.filestage.io/step/**").

const YOUTUBE_ID = /^(?:https?:\/\/)?(?:www\.)?(?:youtube\.com\/watch\?(?:.*&)?v=|youtu\.be\/)([\w-]{11})(?:[?&#].*)?$/;

const getUrl = (node) => {
  if (node.type === 'link') return node.url;
  if (node.type === 'text') return node.value.trim();
  return null;
};

const embed = (node) => {
  if (node.type === 'paragraph' && node.children && node.children.length === 1) {
    const match = YOUTUBE_ID.exec(getUrl(node.children[0]) || '');
    if (match) {
      return {
        type: 'jsx',
        value:
          '<div className="remark-oembed-inline">' +
          `<iframe src="https://www.youtube.com/embed/${match[1]}" title="YouTube video" ` +
          'frameBorder="0" allow="encrypted-media; picture-in-picture" allowFullScreen loading="lazy" />' +
          '</div>',
      };
    }
  }
  if (node.children) node.children = node.children.map(embed);
  return node;
};

module.exports = () => (tree) => {
  embed(tree);
};
