<article class="node-<?php print $node->nid; ?> <?php print $classes; ?> clearfix"<?php print $attributes; ?>>
  <?php
//    print render($content['field_image']); // TODO: remove (RC legacy)
//    print render($content['field_featured_image']); // TODO: remove (CL legacy)
    print render($content[RSC_LIBRARY_PREFIX.'_featured_image']);
  ?>
  <header>
    <?php print render($title_prefix); ?>
    <h3<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h3>
    <?php print render($title_suffix); ?>
  </header>
  <?php
    hide($content['comments']);
    hide($content['links']);
    print render($content);
  ?>
</article>
