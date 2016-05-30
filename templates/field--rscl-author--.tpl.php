<span class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php
  $authors = array();
  foreach ($items as $delta => $item) {
    $authors[] = render($item);
  }
  print implode(',&nbsp;',$authors);
  ?>
</span>
