<div class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php foreach ($items as $delta => $item): ?>
    <article <?php print $item_attributes[$delta]; ?>><?php print render($item); ?></article>
  <?php endforeach; ?>
</div>